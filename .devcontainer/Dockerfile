ARG DOCKER_TAG=latest
FROM espressif/idf:${DOCKER_TAG}

ARG CONTAINER_USER=esp
ARG USER_UID=1050
ARG USER_GID=$USER_UID
ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

RUN groupadd --gid $USER_GID $CONTAINER_USER \
  && adduser --uid $USER_UID --gid $USER_GID --disabled-password --gecos "" ${CONTAINER_USER} \
  && usermod -a -G root $CONTAINER_USER && usermod -a -G dialout $CONTAINER_USER \
  && apt-get update && apt-get install -y sudo \
  && echo "${CONTAINER_USER} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers \
  && rm -rf /var/lib/apt/lists/*

# install pip and mkdocs tooling in a dedicated venv so we don't modify
# the system-managed Python environment (avoids PEP 668 issues)
USER root
RUN apt-get update \
  && apt-get install -y --no-install-recommends python3-venv python3-pip \
  # create a system venv for documentation tools
  && python3 -m venv /opt/venv \
  && /opt/venv/bin/python -m pip install --upgrade pip setuptools wheel \
  && /opt/venv/bin/pip install --no-cache-dir \
    "mkdocs>=1.5" \
    "mkdocs-material>=9.0" \
    "mkdocstrings>=0.19" \
    pymdown-extensions \
  # make the venv binaries available system-wide via /etc/profile.d
  && echo 'export PATH="/opt/venv/bin:$PATH"' > /etc/profile.d/venv-mkdocs.sh \
  && chmod +x /etc/profile.d/venv-mkdocs.sh \
  && rm -rf /var/lib/apt/lists/*
USER ${CONTAINER_USER}

USER ${CONTAINER_USER}
ENV USER=${CONTAINER_USER}
WORKDIR /home/${CONTAINER_USER}

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash", "-c"]