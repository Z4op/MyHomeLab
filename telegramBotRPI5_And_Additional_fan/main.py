from telegram.ext import ApplicationBuilder, CommandHandler, ContextTypes
from telegram import Update
import subprocess
import serial
import docker
import os

valore = os.environ["TOKEN"]

# --------------------------------------------------------------------------------
# Saluto al bot
# --------------------------------------------------------------------------------
async def hello(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    await update.message.reply_text(f"Hello {update.effective_user.first_name}")


# --------------------------------------------------------------------------------
# controllo della temperatura
# --------------------------------------------------------------------------------
async def show_temp(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    # Read temperature
    with open("/sys/class/thermal/thermal_zone0/temp", "r") as file:
        temp_value = float(file.read().strip()) / 1000.0

    # Read RPM from serial
    rpm_decode_utf_8 = "N/A"
    try:
        with serial.Serial('/dev/ttyACM0', 115200, timeout=1) as ser:
            # readline() waits for a newline character (\n) instead of reading just 1 byte
            rpm_raw = ser.readline()
            # Decode with error handling to prevent crashes on garbage data
            rpm_decode_utf_8 = rpm_raw.decode('utf-8', errors='replace').strip()
    except Exception as e:
        rpm_decode_utf_8 = f"Errore lettura seriale: {e}"

    messaggio = (
        f"🌡️ La temperatura corrente è: {temp_value:.1f}°C\n"
        f"🌀 La velocità della ventola è: {rpm_decode_utf_8} rpm"
    )

    await update.message.reply_text(messaggio)

# --------------------------------------------------------------------------------
# controllo dello stato di NGINX
# --------------------------------------------------------------------------------
async def check_nginx(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    # Esegue il comando e cattura l'output
    result = subprocess.run(
        ['systemctl', 'is-active', 'nginx'],
        capture_output=True,
        text=True,
        check=False  # Non solleva eccezione se il servizio è inattivo
    )
    
    stato = result.stdout.strip()
    
    # Formatta il messaggio in base allo stato
    if stato == 'active':
        messaggio = "✅ Nginx è *attivo* e funzionante."
    elif stato == 'inactive':
        messaggio = "⚠️ Nginx è *inattivo*."
    elif stato == 'failed':
        messaggio = "❌ Nginx è *fallito*. Controlla i log!"
    else:
        messaggio = f"ℹ️ Stato nginx: *{stato}*"
    
    # Invia il messaggio
    await update.message.reply_text(messaggio, parse_mode='Markdown')

# --------------------------------------------------------------------------------
# controllo dello stato di DOCKER
# --------------------------------------------------------------------------------
async def container_status(update: Update, context: ContextTypes.DEFAULT_TYPE) -> None:
    messaggio = ""
    client = docker.from_env()

    # Container in esecuzione
    running_containers = client.containers.list()

    messaggio += f"Container in esecuzione: {len(running_containers)}\n"

    for container in running_containers:
        info = container.attrs

        if("running" in info['State']['Status']):
            messaggio += f"🟩 **{container.name}** -> {info['State']['Status']}\n Ports: {info['NetworkSettings']['Ports']}"

        else:
            messaggio += f"🟥 {container.name}"

        messaggio += "\n\n"
        #print(f"📦 {container.name}")
        #print(f"   ID: {container.short_id}")
        #print(f"   Immagine: {info['Config']['Image']}")
        #print(f"   Stato: {info['State']['Status']}")
        #print(f"   Creato: {info['Created']}")

        # Porte esposte
        #if info['NetworkSettings']['Ports']:
        #    print(f"   Porte: {info['NetworkSettings']['Ports']}")

    await update.message.reply_text(messaggio, parse_mode='Markdown')


#print(valore)
app = ApplicationBuilder().token(valore).build()
app.add_handler(CommandHandler("hello", hello))
app.add_handler(CommandHandler("temp", show_temp))
app.add_handler(CommandHandler("proxy", check_nginx))
app.add_handler(CommandHandler("docker", container_status))

app.run_polling()
