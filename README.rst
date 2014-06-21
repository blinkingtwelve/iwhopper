
==================
iwhopper
==================

About
-----

This is a GPLv2 (see ``LICENSE`` file) Linux WiFi channel hopper based on ``iwconfig`` from `iw <http://wireless.kernel.org/en/users/Documentation/iw/>`_ by Jean Tourrilhes.
It contains an internal copy of parts of iw_.

You might want to use this in combination with tcpdump, tshark or wireshark.

Author
------

Wicher Minnaard <wicher@nontrivialpursuit.org>

Usage
------
::

    iwhopper INTERFACE DWELLTIME CHANNELS [LOCKFILE]

For example::

    iwhopper wlan0 2.5 1,2,2,2,2,3 /tmp/somelockfile


This will spend 2.5 seconds on channel 1, 10 seconds on channel 2, and 2.5 seconds on channel 3 — then starts all over.
Running multiple instances of iwhopper with the same ``LOCKFILE`` will prevent them from changing channels concurrently — use
this when your hardware/driver can't handle that, as is sometimes the case when multiple PHYs are attached to the same chip in dualband hardware.

Compiling & Installing
----------------------


Simply running::

    make

will result in a static ``iwhopper`` binary. For more options, review the ``Makefile``.
For instance, if you want to cross-compile with some OpenWRT buildroot toolchain::

    CROCO="/path/to/your/openwrt/staging_dir/toolchain-mips_gcc-4.6-linaro_uClibc-0.9.33.2/bin/mips-openwrt-linux-" make

