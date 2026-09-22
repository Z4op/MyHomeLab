openssl req -new -newkey rsa:2048 -nodes \
  -keyout homelab.key \
  -out homelab.csr \
  -subj "/C=IT/O=HomeLab/CN=*.pi" \
  -addext "subjectAltName=DNS:forgejo.pi,DNS:nextcloud.pi,DNS:jellyfin.pi,DNS:immich.pi,DNS:pihole.pi,DNS:grafana.pi,DNS:scope.pi,DNS:vault.pi,DNS:notes.pi"
