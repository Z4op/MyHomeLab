In my SetUp I have a raspberry pi 5 with Armbian as primary operative system, and  I add external hard-disk, so I have more space for every thing I what to do or I want to try.

In my case I have 2 HDD of 1TB, 1 SSHD of 2TB and an NVME of 128GB where I want to try the IA model compression.
![[HomeLabe.png]]

# Docker services
I have the current service
- **vaultWarden**
	- password manager opensource
- **Forgejo**
	- Personal github server one of the most lightweight around
- **jellyfin**
	- Local Streaming server 
- **pihole**
	- Local DNS server with ad blocker allready build in
	- List of Record:
		- hole.pi -> 192.168.1.2
		- notes.pi -> 192.168.1.2
		- vault.pi -> 192.168.1.2
		- scope.pi ->  192.168.1.2
		- forgejo.pi -> 192.168.1.2
		- fw.pi -> 192.168.1.2
- **trilium**
	-  A note taker application
- **dockerscope**
	-  A tool to check the status of the docker container
- **iptables-web-ui**
	- software to manage e verify the 

# Reverse Proxy
I used nginx as my reverse-proxy, he manage all the direction
- server_name https://hole.pi -> proxy_pass http://localhost:8811
- server_name https://jellyfin.pi -> proxy_pass http://localhost:8096
- server_name https://forgejo.pi -> proxy_pass http://localhost:3000
- server_name https://vault.pi -> proxy_pass http://localhost:80
- server_name https://notes.pi -> proxy_pass http://localhost:8080
- server_name https://scope.pi -> proxy_pass http://localhost:4681
- server_name https://fw.pi -> proxy_pass http://localhost:8585

# firewall configuration
## iptables
here to modify the Docker rule
```bash
modification the table with the rule of ipv4/DOCKER-FORWARD
from:
-i br-fedfabccf643 -j ACCEPT

to:
-i br-fedfabccf643 -j DROP 
# disabling the forward of the forgejo interface so now it is open only localy


modify the table ipv4/NAT/DOCKER to disable the forward of the pihole interface and the nat of the 54 port to the 5335
from:
! -i br-a307629ae4e8 -p tcp -m tcp --dport 8811 -j DNAT --to-destination 172.19.0.2:80
! -i br-a307629ae4e8 -p tcp -m tcp --dport 5335 -j DNAT --to-destination 172.19.0.3:5335 #tcp
! -i br-a307629ae4e8 -p udp -m udp --dport 5335 -j DNAT --to-destination 172.19.0.3:5335 #udp

to:
-d 127.0.0.1/32 ! -i br-a307629ae4e8 -p tcp -m tcp --dport 8811 -j DNAT --to-destination 172.19.0.2:80
-d 127.0.0.1/32 ! -i br-a307629ae4e8 -p tcp -m tcp --dport 5335 -j DNAT --to-destination 172.19.0.3:5335 #tcp 
-d 127.0.0.1/32 ! -i br-a307629ae4e8 -p udp -m udp --dport 5335 -j DNAT --to-destination 172.19.0.3:5335 #udp


```

## UFW
To manage in simpler way the default rule of the machine
```bash
sudo ufw allow in on end0 to any port 22 proto tcp
sudo ufw allow in on tailscal0 to any port 22 proto tcp
sudo ufw allow in on end0 to any port 443 proto tcp
sudo ufw allow in on tailscl0 to any port 443 proto tcp
sudo ufw allow in on end0 to any port 53 proto tcp
sudo ufw allow in on tailscl0 to any port 53 proto tcp

sudo ufw allow in on end0 from 192.168.1.0/24 to any port 139 proto tcp
sudo ufw allow in on end0 from 192.168.1.0/24 to any port 445 proto tcp
```