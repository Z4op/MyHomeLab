# Description
this is a services that manage the extra fan that I use to help cooling out my raspberry pi 5


![[Elecrticall_Sheet.png]]

# 1.Installa le dipendenze
```bash
sudo apt update -y && sudo apt upgrade -y
sudo apt install g++
```

# 2.Compile the source code
```bash
g++ fan-control.c++ -o fan-control
```

# 3.Put the source code in the right place and make it executable
```bash
sudo cp fan-control /usr/local/bin
sudo chmod +x /usr/local/bin/fan-control
```

# 4.Copy the servies
```bash
sudo cp fan-control.service fan-control-start.timer fan-control-stop.timer /etc/systemd/system/
```

# 5.Activate all services
```bash
sudo systemctl enable --now fan-control.service
sudo systemctl enable --now fan-control-start.timer
sudo systemctl enable --now fan-control-stop.timer
```
