import argparse
import subprocess
import yaml
import os


def _exec_cmd(cmd, verbose=True, wait=True):
    print(cmd)
    with subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE) as proc:
        if wait:
            res, err = proc.communicate()
            return_code = proc.returncode
            if return_code != 0:
                print("Error: ")
                print(err.decode("utf-8"))
                exit(1)
            elif verbose:
                print(res.decode("utf-8"))

def clean(config):
    rootdir = os.getcwd()
    for target in config["targets"]:
        os.chdir(rootdir)
        
        name = list(target.keys())[0]
        if "path" in target[name]:
            path = target[name]["path"]
        else:
            path = f"nodes/{name}"
        print(f"Cleaning: {path}")
        
        # create a build folder
        build_folder = f"{rootdir}/{path}/build"
        dist_folder = f"{rootdir}/{path}/dist"
        _exec_cmd(["rm", "-rf", build_folder], verbose=False)
        _exec_cmd(["rm", "-rf", dist_folder], verbose=False)

def build(config):
    rootdir = os.getcwd()
    for target in config["targets"]:
        os.chdir(rootdir)
        
        name = list(target.keys())[0]
        if "path" in target[name]:
            path = target[name]["path"]
        else:
            path = f"nodes/{name}"
        print("\n" + "- "*30)
        print(f"Building: {path}")
        
        # create a build folder
        build_folder = f"{rootdir}/{path}/build"
        print(f"Creating build folder at {build_folder}")
        _exec_cmd(["mkdir", "-p", build_folder])
        os.chdir(build_folder)
        
        # run cmake
        cmd = ["cmake"]
        # static aruguments
        cmd.append(f"-DCMAKE_TOOLCHAIN_FILE={rootdir}/common/cmake/clang_toolchain.cmake")
        cmd.append("")
        # figure out config arguments
        if "build_type" in target[name]:
            cmd.append(f"-DBUILD_TYPE={target[name]['build_type']}")
        if "build_test" in target[name]:
            cmd.append(f"-DBUILD_TESTS={target[name]['build_tests']}")
        if "build_arm" in target[name]:
            cmd.append(f"-DBUILD_ARM={target[name]['build_arm']}")
        # specify the root of the node (we are currently in build folder)
        cmd.append("..")
        _exec_cmd(cmd)
        
        # run make
        _exec_cmd(["make", "-j8"])
        if "install" in target[name] and target[name]["install"]:
            _exec_cmd(["make", "install"])
    os.chdir(rootdir)

def run(config):
    xpos = ypos = 0
    for target in config["targets"]:
        name = list(target.keys())[0]
        if target[name]["run"]:
            if "path" in target[name]:
                path = target[name]["path"]
            else:
                path = f"nodes/{name}"
            print("\n" + "- "*30)
            print(f"Running: {path}")
            exec_path = f"{path}/build/{name}"
            xpos += 20
            ypos += 20
            _exec_cmd(["gnome-terminal", f"--title={name}", "--geometry", f"73x9+{xpos}+{ypos}", "--", exec_path])

def main():
    parser = argparse.ArgumentParser(description="Building multiple targets (nodes, common modules, dependencies, etc.)")
    parser.add_argument("--config_path", type=str, default="scripts/prj_config.yml", help="Config yml file to specificy what to build")
    parser.add_argument("-c", "--clean", action="store_true", help="Clean everything before building")
    parser.add_argument("-r", "--run", action="store_true", help="Run everything after building")
    args = parser.parse_args()
    
    print(f"Loading build config from: {args.config_path}")
    with open(args.config_path, "r") as f:
        config = yaml.load(f, Loader=yaml.SafeLoader)
        if args.clean:
            clean(config)
        build(config)
        if args.run:
            run(config)

if __name__ == "__main__":
    main()
