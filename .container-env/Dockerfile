FROM ubuntu:24.04

ARG ZSDK_VERSION=0.16.4
ARG PYTHON_VERSION=3.12

ARG UID=1001
ARG GID=1001

#
# --- General ---
#

ENV TZ=Europe/Berlin
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update \
 && apt-get upgrade -y \
 && apt-get clean \
 && apt-get install -y software-properties-common

RUN apt update

RUN apt-get install -y sudo bash-completion vim nano man-db less inotify-tools libncurses6 \
  && apt-get clean

# Avoid pwd for sudo
RUN echo "%sudo ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/sudo-nopasswd

#
# --- Zephyr ---
#
RUN apt-get install -y --no-install-recommends git ninja-build gperf \
  ccache dfu-util device-tree-compiler wget clang-format \
  python3-pip python3-setuptools python3-wheel python3-venv python${PYTHON_VERSION}-tk python${PYTHON_VERSION}-dev \
  xz-utils file make gcc gcc-multilib g++-multilib libsdl2-dev pkg-config cmake iproute2 openocd iptables ruby ssh \
  xvfb bzip2 dos2unix sudo unzip clang-tidy cppcheck clang\
  && apt-get clean


RUN mkdir -p /opt

# Zephyr SDK toolchain
RUN wget -q --show-progress --progress=bar:force:noscroll https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZSDK_VERSION}/zephyr-sdk-${ZSDK_VERSION}_linux-x86_64_minimal.tar.xz && \
    wget -O - https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${ZSDK_VERSION}/sha256.sum | shasum --check --ignore-missing && \
    tar xvf zephyr-sdk-${ZSDK_VERSION}_linux-x86_64_minimal.tar.xz -C /opt/ && \
    rm zephyr-sdk-${ZSDK_VERSION}_linux-x86_64_minimal.tar.xz && \
    cd /opt/zephyr-sdk-${ZSDK_VERSION} && \
    ./setup.sh -t x86_64-zephyr-elf -t arm-zephyr-eabi -h

#
# --- NRF command line tools ---
#
RUN wget https://nsscprodmedia.blob.core.windows.net/prod/software-and-other-downloads/desktop-software/nrf-command-line-tools/sw/versions-10-x-x/10-15-1/nrf-command-line-tools-10.15.1_linux-amd64.zip \
    && unzip nrf-command-line-tools-10.15.1_linux-amd64.zip \
    && dpkg -i --force-overwrite nrf-command-line-tools_10.15.1_amd64.deb \
    && dpkg -i --force-overwrite JLink_Linux_V758b_x86_64.deb \
    && rm nrf-command-line-tools-10.15.1_linux-amd64.zip \
    && rm JLink_Linux_V758b_x86_64.deb \
    && rm nrf-command-line-tools-10.15.1_Linux-amd64.tar.gz \
    && rm JLink_Linux_V758b_x86_64.tgz \
    && rm nrf-command-line-tools-10.15.1-1.amd64.rpm \
    && rm nrf-command-line-tools_10.15.1_amd64.deb

RUN apt-get install -y minicom

# #
# # --- ENVIRONMENT ---
# #

#Dont set ZEPHYR_BASE as it is dependent on the west_workspace
#ENV WEST_WORKSPACE_CONTAINER=/opt/zephyrproject
#ENV ZEPHYR_BASE=$WEST_WORKSPACE_CONTAINER/zephyr
ENV PKG_CONFIG_PATH=/usr/lib/i386-linux-gnu/pkgconfig
ENV ZEPHYR_TOOLCHAIN_PATH=/opt/zephyr-sdk-${ZSDK_VERSION}

#
# Remove 'ubuntu' account
#
RUN userdel -r ubuntu

#
# Create 'user' account
#
RUN groupadd -g $GID -o user

RUN mkdir -p /etc/sudoers.d && useradd -u $UID -m -g user -G plugdev user \
	&& echo 'user ALL = NOPASSWD: ALL' > /etc/sudoers.d/user \
	&& chmod 0440 /etc/sudoers.d/user

RUN usermod -a -G dialout user

RUN mkdir -p /opt/zephyrproject/ && sudo chown -R user:user /opt/zephyrproject/

# Clean up stale packages
RUN apt-get clean -y && \
	apt-get autoremove --purge -y && \
	rm -rf /var/lib/apt/lists/*

# Add entrypoint script
ADD ./entrypoint.sh /home/user/entrypoint.sh
RUN dos2unix /home/user/entrypoint.sh && chmod +x /home/user/entrypoint.sh

RUN chsh --shell /bin/bash user

USER user

ENTRYPOINT ["/home/user/entrypoint.sh"]
