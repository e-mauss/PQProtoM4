import subprocess
import os.path as osp
import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-p', '--proto', nargs='?', default="bench_OCAKE")           # positional argument
parser.add_argument('-k', '--kem', nargs='?', default="ml-kem-512")      # option that takes a value
parser.add_argument('-i', '--impl', nargs='?', default="")  # on/off flag
args = parser.parse_args()

file_path = osp.dirname(os.path.realpath(__file__))
client_dir = osp.join(file_path, "client")
server_dir = osp.join(file_path, "server")

mk = subprocess.Popen(["make", "clean"], cwd=server_dir, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
mk.communicate()
mk = subprocess.Popen(["make", "clean"], cwd=client_dir, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
mk.communicate()
mk = subprocess.Popen(["make", f"KEM={args.kem}", f"PROTO={args.proto}", f"IMPL={args.impl}"], cwd=server_dir, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
mk.communicate()
mk = subprocess.Popen(["make", f"KEM={args.kem}", f"PROTO={args.proto}", f"IMPL={args.impl}"], cwd=client_dir, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
mk.communicate()

server=subprocess.Popen(["./bench_server", "/dev/ttyACM0"], cwd=server_dir, stderr=subprocess.STDOUT)
client=subprocess.call(["make", "flash", f"KEM={args.kem}", f"PROTO={args.proto}", f"IMPL={args.impl}"], cwd=client_dir, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)
server.communicate()
