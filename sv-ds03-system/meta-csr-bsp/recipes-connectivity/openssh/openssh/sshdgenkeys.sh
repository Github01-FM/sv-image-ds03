#!/bin/sh

mkdir -p /var/lib/ssh
ssh-keygen -t rsa -P '' -f /var/lib/ssh/ssh_host_rsa_key
ssh-keygen -t dsa -P '' -f /var/lib/ssh/ssh_host_dsa_key
ssh-keygen -t ecdsa -P '' -f /var/lib/ssh/ssh_host_ecdsa_key
