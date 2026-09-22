#!/bin/bash
openssl x509 -req \
  -in homelab.csr \
  -CA homelab-ca.crt \
  -CAkey homelab-ca.key \
  -CAcreateserial \
  -out homelab.crt \
  -days 825 \
  -sha256 \
  -extfile san.cnf
