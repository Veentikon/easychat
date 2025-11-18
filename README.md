# EasyChat

A simple, secure communication channel using Tailscale and SSH.

EasyChat provides a lightweight and private way for users to communicate over a secure network without complex networking setup.
By leveraging Tailscale for mesh VPN connectivity and SSH for encrypted terminal messaging, EasyChat makes it easy to deploy a trusted communication environment.

## Features

🔒 Secure, encrypted communication over SSH

🌐 Zero-config networking using Tailscale mesh VPN

👥 Multiple user accounts with access control

🖥️ Terminal-based messaging via the write command

⚙️ Optional C client/server program for streamlined messaging (docs coming soon)

## Requirements
Hardware / Software

A Linux server (any distribution)

Linux client machines for each user

Tailscale installed on all machines

SSH (OpenSSH)

## Optional

C compiler (e.g., gcc) for the enhanced client/server program

1. Install Linux on the Server

Install any Linux distribution you prefer.
Example: an Ubuntu Server VM running inside Proxmox.

2. Install Tailscale

Install Tailscale on both the server and all client devices:

Linux (Debian/Ubuntu example):

curl -fsSL https://tailscale.com/install.sh | sh
sudo tailscale up


Once authenticated, each device automatically joins your Tailscale network and receives a private Tailscale IP.

Users and devices can easily be shared or revoked from the Tailscale admin panel.

3. Create User Accounts

Each EasyChat participant needs a normal Linux user account on the server.

sudo useradd -m <username>
sudo passwd <username>


This ensures each user has their own environment and permissions.

4. Install & Configure SSH

Install OpenSSH if it's not already present:

sudo apt install openssh-server


Edit the SSH configuration file:

sudo nano /etc/ssh/sshd_config

Password Authentication Options
Option A — Password Authentication (Least Secure)

Uncomment the following lines:

PasswordAuthentication yes
PermitEmptyPasswords no


⚠️ Vulnerable to brute-force attacks.
Recommended only if access is via Tailscale and users are trusted.

Option B — SSH Key Authentication (Recommended)

Set:

PasswordAuthentication no


This disables password logins entirely and relies on cryptographic key pairs.

Restart SSH after editing:

sudo systemctl restart ssh

5. Set Up SSH Key Authentication (Recommended)
Generate a key pair:
ssh-keygen -t rsa -b 4096


You may choose a custom filename to avoid overwriting existing keys.

Copy the public key to the server:
ssh-copy-id <username>@<server-tailscale-ip>


After this, login will work via SSH keys only.

6. Enable Message Reception via write

The write command allows one user to send text directly to another user's terminal.

Modern systems often disable this by default for security.

Enable it (per user):

mesg y


⚠️ Security Note:
The write command has historical permission vulnerabilities.
Only enable it for trusted users and avoid enabling it on the root account.

7. Sending Messages Between Users

To send a message to a logged-in user:

write <username>


Press ENTER, type your message, and finish with:

Ctrl + D


The recipient immediately sees the message in their terminal.

How It Works

EasyChat relies on:

Tailscale → provides a secure, private network between all participating devices

SSH → encrypts communication between users and the server

Linux messaging utilities → provide simple and instant communication

This combination offers a reliable channel without the complexity of configuring firewalls, port forwarding, or public IPs.

Future Enhancements

This project includes (or will include) a custom C client/server program for more streamlined and user-friendly messaging.
Documentation for the enhanced messaging system will be added later.

## Security Considerations

### Only grant access to trusted users.

### Prefer SSH key authentication over passwords.

### Never enable mesg y for root.

### Use Tailscale ACLs to control user access.

### Consider restricting SSH to Tailscale’s interface only.

## License

MIT

## Feature Enhancements

#### Enhanced Chat (Optional Module)

EasyChat includes an additional C-based socket chat application that provides a more streamlined, interactive messaging experience.

The enhanced module is located in:

#### enhanced-chat/

This module contains:

chatclient.c – TCP client

chatserver.c – TCP server

chatroom.h – shared header

Makefile – build automation

plan.txt – initial design notes & usage examples

## Building the Enhanced Chat App

From inside the enhanced-chat directory:

make

This compiles both the client and server binaries.

#### Usage

_./chatserver_
_./chatclient <username>_

##### More detailed documentation is included in plan.txt and will be expanded in a dedicated README.

## Contributing

Contributions, ideas, and enhancements are welcome!
Feel free to submit issues or pull requests.
