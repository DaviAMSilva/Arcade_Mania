# SOURCE: https://devkitpro.org/wiki/devkitPro_pacman

#!/usr/bin/env bash
if ! [ $(id -u) = 0 ]; then
  echo "Need root privilege to install!"
  exit 1
fi

# ensure apt is set up to work with https sources
sudo apt-get install apt-transport-https -y

# Store devkitPro gpg key locally if we don't have it already
if ! [ -f /usr/local/share/keyring/devkitpro-pub.gpg ]; then
  mkdir -p /usr/local/share/keyring/
  wget -O /usr/local/share/keyring/devkitpro-pub.gpg https://apt.devkitpro.org/devkitpro-pub.gpg
fi

# Add the devkitPro apt repository if we don't have it set up already
if ! [ -f /etc/apt/sources.list.d/devkitpro.list ]; then
  echo "deb [signed-by=/usr/local/share/keyring/devkitpro-pub.gpg] https://apt.devkitpro.org stable main" > /etc/apt/sources.list.d/devkitpro.list
fi

# Finally install devkitPro pacman
sudo apt-get update
sudo apt-get install devkitpro-pacman -y



# Install gba-dev
echo -e "6 7 10 12 14 \ny" | sudo dkp-pacman -S gba-dev