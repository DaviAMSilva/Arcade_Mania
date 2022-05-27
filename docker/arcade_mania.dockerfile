# Repositório: https://hub.docker.com/r/daviamsilva/arcade_mania
FROM devkitpro/devkitarm

# Usado para criar as releases automáticas
RUN sudo apt-get update
RUN sudo apt-get install -y nodejs