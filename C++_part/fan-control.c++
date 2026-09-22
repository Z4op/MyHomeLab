#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// Configurazione
const std::string SERIAL_PORT = "/dev/ttyACM0"; // Modifica se il tuo RP2040 è su ttyACM1 o ttyUSB0
const int CHECK_INTERVAL_SEC = 5;               // Controllo ogni 5 secondi

// Legge la temperatura della CPU in gradi Celsius
double getCpuTemperature() {
    std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
    double temp = 0;
    if (file.is_open()) {
        file >> temp;
        temp /= 1000.0; // Converti da milli-Celsius a Celsius
    } else {
        std::cerr << "Impossibile leggere la temperatura. Verifica il percorso." << std::endl;
    }
    return temp;
}

// Calcola il valore PWM (0-255) in base alla temperatura
int calculatePWM(double temp) {
    // Cap massimo all'80% (255 * 0.8 = 204) per non arrivare mai a piena potenza
    const int MAX_PWM = 204; 
    
    if (temp <= 45.0) {
        return 51; // ~20% (minimo per evitare stallo della ventola)
    }
    if (temp >= 60.0) {
        return MAX_PWM; // Hard cap a 80%
    }
    
    // Interpolazione lineare tra 45°C e 50°C (da 51 a 127)
    if (temp <= 50.0) {
        double ratio = (temp - 45.0) / (50.0 - 45.0);
        return std::round(51 + ratio * (127 - 51));
    }
    
    // Interpolazione lineare tra 50°C e 60°C (da 127 a 204)
    double ratio = (temp - 50.0) / (60.0 - 50.0);
    return std::round(127 + ratio * (MAX_PWM - 127));
}

// Invia il comando seriale all'RP2040
bool sendFanCommand(int pwm_value) {
    int fd = open(SERIAL_PORT.c_str(), O_WRONLY | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        return false; // Porta non disponibile (es. RP2040 disconnesso)
    }
    
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        close(fd);
        return false;
    }
    
    // Configurazione 115200 baud, 8N1
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 5;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CRTSCTS);
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        return false;
    }
    
    std::string cmd = std::to_string(pwm_value) + "\n";
    write(fd, cmd.c_str(), cmd.length());
    close(fd);
    return true;
}

int main() {
    std::cout << "Avvio del controllo ventola RP2040..." << std::endl;
    
    while (true) {
        double temp = getCpuTemperature();
        int pwm = calculatePWM(temp);
        
        if (sendFanCommand(pwm)) {
            std::cout << "[OK] Temp: " << temp << "°C -> PWM: " << pwm 
                      << " (" << std::round((pwm / 255.0) * 100) << "%)" << std::endl;
        } else {
            std::cerr << "[ERRORE] Impossibile scrivere su " << SERIAL_PORT 
                      << ". Verifica la connessione USB." << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(CHECK_INTERVAL_SEC));
    }
    
    return 0;
}
