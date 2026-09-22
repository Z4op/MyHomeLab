openssl req -x509 -newkey rsa:4096 -sha256 -days 3650 -nodes \
  -keyout homelab-ca.key \
  -out homelab-ca.crt \
  -subj "/C=IT/O=HomeLab/CN=HomeLab Root CA"
