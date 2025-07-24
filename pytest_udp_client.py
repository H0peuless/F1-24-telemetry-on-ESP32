# SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: Apache-2.0
import logging
import socket

import pytest
from common_test_methods import get_env_config_variable
from common_test_methods import get_host_ip4_by_dest_ip
from common_test_methods import get_host_ip6_by_dest_ip
from common_test_methods import get_my_interface_by_dest_ip
from pexpect.exceptions import TIMEOUT
from pytest_embedded import Dut

try:
    from run_udp_server import UdpServer
except ImportError:
    import os
    import sys
    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'scripts')))
    from run_udp_server import UdpServer


PORT = 20777
MAX_RETRIES = 3


@pytest.mark.esp32
@pytest.mark.esp32s2
@pytest.mark.esp32c2
@pytest.mark.esp32c3
@pytest.mark.esp32s3
@pytest.mark.esp32c5
@pytest.mark.esp32c6
@pytest.mark.esp32c61
@pytest.mark.wifi_router
def test_examples_udp_client_ipv4(dut: Dut) -> None:
    # Parse IP address of STA
    logging.info('Waiting to connect with AP')
    env_name = 'wifi_router'
    ap_ssid = get_env_config_variable(env_name, 'Freebox-756C92')
    ap_password = get_env_config_variable(env_name, 'adflictet!-rufinianum&-dificabam-vivent2')
    dut.write(f'{ap_ssid} {ap_password}')
    
    ipv4 = dut.expect(r'IPv4 address: (\d+\.\d+\.\d+\.\d+)[^\d]', timeout=30)[1].decode()
    print(f'Connected with IPv4={ipv4}')

    # test IPv4
    with UdpServer(PORT, socket.AF_INET):
        server_ip = get_host_ip4_by_dest_ip(ipv4)
        print('Connect udp client to server IP={}'.format(server_ip))
        for _ in range(MAX_RETRIES):
            try:
                dut.write(server_ip)
                dut.expect('OK: Message from ESP32')
                break
            except TIMEOUT:
                pass
        else:
            raise ValueError('Failed to send/recv udp packets.')
