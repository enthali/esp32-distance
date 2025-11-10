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
  && apt-get update && apt-get install -y \
    sudo \
    iproute2 \
    iputils-ping \
    net-tools \
    netcat-openbsd \
  && echo "${CONTAINER_USER} ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers \
  && rm -rf /var/lib/apt/lists/*

# install pip and documentation tooling in a dedicated venv so we don't modify
# the system-managed Python environment (avoids PEP 668 issues)
USER root
RUN apt-get update \
  && apt-get install -y --no-install-recommends python3-venv python3-pip nodejs npm doxygen graphviz libgraphviz-dev \
  # install XFCE4 desktop environment for GUI support
  && apt-get install -y --no-install-recommends xfce4 xfce4-goodies \
  # set German keyboard layout as default (can be changed later)
  && echo 'XKBLAYOUT="de"' >> /etc/default/keyboard \
  && echo 'XKBVARIANT=""' >> /etc/default/keyboard \
  && echo 'XKBOPTIONS=""' >> /etc/default/keyboard \
  # create a system venv for documentation tools
  && python3 -m venv /opt/venv \
  && /opt/venv/bin/python -m pip install --upgrade pip setuptools wheel \
  && /opt/venv/bin/pip install --no-cache-dir \
    "sphinx>=8.0" \
    "sphinx-needs>=6.0" \
    "sphinx-rtd-theme>=3.0" \
    "furo>=2025.0" \
    "myst-parser>=4.0" \
    "sphinxcontrib-plantuml>=0.31" \
  # install markdownlint-cli for markdown validation
  && npm install -g markdownlint-cli \
  # make the venv binaries available system-wide via /etc/profile.d
  && echo 'export PATH="/opt/venv/bin:$PATH"' > /etc/profile.d/venv-sphinx.sh \
  && chmod +x /etc/profile.d/venv-sphinx.sh \
  && rm -rf /var/lib/apt/lists/*
USER ${CONTAINER_USER}

USER ${CONTAINER_USER}
ENV USER=${CONTAINER_USER}
WORKDIR /home/${CONTAINER_USER}

# Create specific temp directory and set TMPDIR
#RUN mkdir -p /home/${CONTAINER_USER}/tmp \
#  && chmod 755 /home/${CONTAINER_USER}/tmp

# Set TMPDIR environment variable to avoid /tmp permission issues
#ENV TMPDIR=/home/${CONTAINER_USER}/tmp

RUN echo "source /opt/esp/idf/export.sh > /dev/null 2>&1" >> ~/.bashrc

ENTRYPOINT [ "/opt/esp/entrypoint.sh" ]

CMD ["/bin/bash", "-c"]