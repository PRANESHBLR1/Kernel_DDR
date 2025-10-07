#!/usr/bin/env bash
mkdir -p certs
cd certs

openssl req -x509 -newkey rsa:2048 -nodes -keyout server.key -out server.crt \
  -subj "/CN=127.0.0.1" -days 365
openssl req -x509 -newkey rsa:2048 -nodes -keyout client.key -out client.crt \
  -subj "/CN=client" -days 365
