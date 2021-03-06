#!/usr/bin/env python3


#
#    Copyright (c) 2016-2017 Nest Labs, Inc.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    @file
#       This file calls Weave BDX tests with different sizes of files and different BDX versions:
#          -- download, upload
#          -- file size in 0, 10, 100, 1000 bytes
#          -- BDX versions in BDX-v0, BDX-v1, and BDX-v0&v1 interop
#

from __future__ import absolute_import
from __future__ import print_function
import filecmp
import itertools
import os
import random
import shutil
import string
import unittest
import set_test_path

from happy.Utils import *
import happy.HappyNodeList
import WeaveStateLoad
import WeaveStateUnload
import WeaveBDX
import WeaveUtilities
from six.moves import range

class test_weave_bdx_01(unittest.TestCase):
    def setUp(self):
        self.tap = None

        if "WEAVE_SYSTEM_CONFIG_USE_LWIP" in list(os.environ.keys()) and os.environ["WEAVE_SYSTEM_CONFIG_USE_LWIP"] == "1":
            self.topology_file = os.path.dirname(os.path.realpath(__file__)) + \
                "/../../../topologies/standalone/three_nodes_on_tap_thread_weave.json"
            self.tap = "wpan0"
        else:
            self.topology_file = os.path.dirname(os.path.realpath(__file__)) + \
                "/../../../topologies/standalone/three_nodes_on_thread_weave.json"

        self.show_strace = False

        # setting Mesh for thread test
        options = WeaveStateLoad.option()
        options["quiet"] = True
        options["json_file"] = self.topology_file

        setup_network = WeaveStateLoad.WeaveStateLoad(options)
        ret = setup_network.run()


    def tearDown(self):
        # cleaning up
        options = WeaveStateUnload.option()
        options["quiet"] = True
        options["json_file"] = self.topology_file

        teardown_network = WeaveStateUnload.WeaveStateUnload(options)
        teardown_network.run()


    def test_weave_bdx_download(self):
        self.test_num = 0
        bdx_version = [0, 1]
        pairs_of_versions = list(itertools.product(bdx_version, bdx_version))

        for file_size in [0, 10, 100, 1000]:
            for version in pairs_of_versions:
                self.__weave_bdx("download", file_size, version[0], version[1])
                self.__weave_bdx("upload", file_size, version[0], version[1])


    def __weave_bdx(self, direction, file_size, server_version, client_version):
        options = happy.HappyNodeList.option()
        options["quiet"] = True

        if direction == "download":
            value, data = self.__run_bdx_download_between("node01", "node02", file_size, server_version, client_version)
        else:
            value, data = self.__run_bdx_upload_between("node01", "node02", file_size, server_version, client_version)

        self.__process_result("node01", "node02", value, data, direction, file_size, server_version, client_version)
        self.test_num += 1


    def __process_result(self, nodeA, nodeB, value, data, direction, file_size, server_version, client_version):

        if direction == "download":
            print("bdx download of " + str(file_size) + "B from " + nodeA + "(BDX-v" + str(server_version) + ") to "\
                  + nodeB + "(BDX-v" + str(client_version) + ") ", end=' ')
        else:
            print("bdx upload of " + str(file_size) + "B from " + nodeA + "(BDX-v" + str(server_version) + ") to "\
                  + nodeB + "(BDX-v" + str(client_version) + ") ", end=' ')

        if value:
            print(hgreen("Passed"))
        else:
            print(hred("Failed"))

        try:
            self.assertTrue(value == True, "File Copied: " + str(value))
        except AssertionError as e:
            print(str(e))
            print("Captured experiment result:")

            print("Client Output: ")
            for line in data["client_output"].split("\n"):
                print("\t" + line)

            print("Server Output: ")
            for line in data["server_output"].split("\n"):
                print("\t" + line)

            if self.show_strace:
                print("Server Strace: ")
                for line in data["server_strace"].split("\n"):
                    print("\t" + line)

                print("Client Strace: ")
                for line in data["client_strace"].split("\n"):
                    print("\t" + line)

        if not value:
            raise ValueError("Weave BDX Failed")


    def __run_bdx_download_between(self, nodeA, nodeB, file_size, server_version, client_version):
        test_folder_path = "/tmp/happy_%08d_down_%03d" % (int(os.getpid()), self.test_num)
        test_folder = self.__create_test_folder(test_folder_path)

        server_temp_path = self.__create_server_temp(test_folder)
        test_file = self.__create_file(test_folder, file_size)
        receive_path = self.__create_receive_folder(test_folder)

        options = WeaveBDX.option()
        options["quiet"] = False
        options["server"] = nodeA
        options["client"] = nodeB
        options["tmp"] = server_temp_path
        options["download"] = test_file
        options["receive"] = receive_path
        options["tap"] = self.tap
        options["server_version"] = server_version
        options["client_version"] = client_version

        weave_bdx = WeaveBDX.WeaveBDX(options)
        ret = weave_bdx.run()

        value = ret.Value()
        data = ret.Data()
        copy_success = self.__file_copied(test_file, receive_path)
        self.__delete_test_folder(test_folder)

        return copy_success, data


    def __run_bdx_upload_between(self, nodeA, nodeB, file_size, server_version, client_version):
        test_folder_path = "/tmp/happy_%08d_up_%03d" % (int(os.getpid()), self.test_num)
        test_folder = self.__create_test_folder(test_folder_path)

        server_temp_path = self.__create_server_temp(test_folder)
        test_file = self.__create_file(test_folder, file_size)
        receive_path = self.__create_receive_folder(test_folder)

        options = WeaveBDX.option()
        options["quiet"] = False
        options["server"] = nodeA
        options["client"] = nodeB
        options["tmp"] = server_temp_path
        options["upload"] = test_file
        options["receive"] = receive_path
        options["tap"] = self.tap
        options["server_version"] = server_version
        options["client_version"] = client_version

        weave_bdx = WeaveBDX.WeaveBDX(options)
        ret = weave_bdx.run()

        value = ret.Value()
        data = ret.Data()
        copy_success = self.__file_copied(test_file, receive_path)
        self.__delete_test_folder(test_folder)

        return copy_success, data


    def __file_copied(self, test_file_path, receive_path):
        file_name = os.path.basename(test_file_path)
        receive_file_path = receive_path + "/" + file_name

        if not os.path.exists(receive_file_path):
            print("A copy of a file does not exists")
            return False

        return filecmp.cmp(test_file_path, receive_file_path)


    def __create_test_folder(self, path):
        os.mkdir(path)
        return path


    def __delete_test_folder(self, path):
        shutil.rmtree(path)


    def __create_server_temp(self, test_folder):
        path = test_folder + "/server_tmp"
        os.mkdir(path)
        return path


    def __create_file(self, test_folder, size = 10):
        path = test_folder + "/test_file.txt"
        random_content = ''.join(random.SystemRandom().choice(string.ascii_uppercase + string.digits) for _ in range(size))

        with open(path, 'w+') as test_file:
            test_file.write(random_content)

        return path


    def __create_receive_folder(self, test_folder):
        path = test_folder + "/receive"
        os.mkdir(path)
        return path


if __name__ == "__main__":
    WeaveUtilities.run_unittest()

