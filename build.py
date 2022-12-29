import subprocess
import sys
import os
import json
from platform import system
from shutil import rmtree, copytree, copy, which
import xml.etree.ElementTree as ElementTree
from typing import Dict

if system() not in ['Windows', 'Linux']:
    raise Exception("Unsupported build platform")

# Consts
PLATFORM_CODE = "win" if system() == 'Windows' else 'mac'
CONFIG_FILE: str = './config.json'
DEBUG: bool = '--debug' in sys.argv
BUILD_BINARY: bool = True
# Config init
if os.path.exists(CONFIG_FILE):
    config = json.load(open(CONFIG_FILE, 'r'))
else:
    raise Exception("Config not found")

EXTENSION_VERSION: str = config['version']
EXTENSION_NAME: str = config['name']
EXTENSION_ORG: str = config['org']
EXTENSION_DESC: str = config['description']
EXTENSION_ID: str = f"com.{EXTENSION_ORG}.{EXTENSION_NAME}"
BUILD_FOLDER: str = "./build"
if DEBUG:
    BUILD_FOLDER = os.path.join(BUILD_FOLDER, EXTENSION_ID)

ENV_CONFIG: dict = config['env']

ENV_APPS: Dict[str, str] = ENV_CONFIG['apps']
ENV_LOCALES: list = ENV_CONFIG['locales']
ENV_RUNTIMES: Dict[str, str] = ENV_CONFIG['runtimes']

EXTENSION_INSTANCES: Dict[str, dict] = config['instances']

CSXS_PATH: str = os.path.join(BUILD_FOLDER, "CSXS")


# CPP things
def create_extension_config(path: str,
                            extension_id: str,
                            ui: str,
                            consts: Dict[str, str],
                            class_ids: Dict[str, str]):
    res: str = "#ifndef _PLUGIN_CONFIGURATION_H_\n" \
               "#define _PLUGIN_CONFIGURATION_H_\n" \
               "\n" \
               f'#define PUBLISHER_NAME						"{EXTENSION_NAME}"\n' \
               f'#define PUBLISHER_UNIVERSAL_NAME			"{extension_id}"\n' \
               '\n' \
               f'#define PUBLISH_SETTINGS_UI_ID				"{ui}"\n' \
               '\n' \
               f'#define DOCTYPE_NAME						"{EXTENSION_NAME}"\n' \
               f'#define DOCTYPE_UNIVERSAL_NAME				"{EXTENSION_ID}"\n' \
               f'#define DOCTYPE_DESCRIPTION					"{EXTENSION_DESC}"\n' \
               '\n'

    for constant in consts:
        res += f'#define {constant}     "{consts[constant]}"\n'

    res += f'namespace {EXTENSION_NAME}\n' \
           '{\n'
    for class_id in class_ids:
        res += f'const FCM::FCMCLSID {class_id} = {class_ids[class_id]};\n'

    res += '}\n' \
           '#endif'

    with open(os.path.join(path, 'PluginConfiguration.h'), 'w') as f:
        f.write(res)


def common_extension_processing(name: str, common_filepath: str) -> str:
    if not os.path.exists(common_filepath):
        raise Exception(f"File not exist {common_filepath}")

    src_folder: str = os.path.dirname(common_filepath)
    dst_folder: str = os.path.join(BUILD_FOLDER, name)

    copytree(src_folder, dst_folder)

    return f"./{name}/{os.path.basename(common_filepath)}"


def cpp_building_windows(solution_path: str):
    msbuild = which('msbuild')
    if not msbuild:
        raise Exception('MSBuild not found!')

    print("C++ building..")

    subprocess.run([msbuild, solution_path, f"-property:Configuration={'Release' if not DEBUG else 'Debug'}"])


def cpp_extension_processing(name: str, project_dir: str, cpp_info: dict) -> str:
    # Some settings
    include_files: list = cpp_info['include']

    # Folder init
    extension_folder: str = os.path.join(BUILD_FOLDER, name)
    fcm_folder: str = os.path.join(extension_folder, 'lib')
    os.makedirs(fcm_folder, exist_ok=True)

    solution_path: str = os.path.join(project_dir, "project", PLATFORM_CODE, f"{name}.sln")
    if not os.path.exists(solution_path):
        raise Exception(f"Solution file not found {solution_path}")

    # Project building

    if BUILD_BINARY:
        if PLATFORM_CODE == 'win' and cpp_info['platform']['windows']:
            try:
                cpp_building_windows(solution_path)
            except Exception:
                print("Building failed!")
        #TODO: Mac

        binary_folder = os.path.join(project_dir, 'build', PLATFORM_CODE)
        if os.path.exists(binary_folder):
            copytree(binary_folder, os.path.join(fcm_folder, PLATFORM_CODE))
        else:
            print("Binary files not found. Skip...")

    # Include file copying
    for file in include_files:
        if os.path.isdir(file):
            copytree(file, os.path.join(fcm_folder, os.path.basename(file)), dirs_exist_ok=True)
        else:
            copy(file, fcm_folder)

    open(os.path.join(extension_folder, 'fcm.xml'), 'w+').close()

    return f"./{name}/fcm.xml"


# CEP things
def process_config():
    # Debug
    debug_file: ElementTree.Element = ElementTree.Element("ExtensionList")

    # Manifest
    os.mkdir(CSXS_PATH)
    manifest: ElementTree.Element = ElementTree.Element("ExtensionManifest")
    manifest.attrib['xmlns:xsi'] = 'http://www.w3.org/2001/XMLSchema-instance'
    manifest.attrib['Version'] = "9.0"

    manifest.attrib['ExtensionBundleId'] = str(EXTENSION_ID)
    manifest.attrib['ExtensionBundleVersion'] = str(EXTENSION_VERSION)
    manifest.attrib['ExtensionBundleName'] = str(config['name'])

    # Main elements
    extension_list: ElementTree.Element = ElementTree.SubElement(manifest, 'ExtensionList')

    env: ElementTree.Element = ElementTree.SubElement(manifest, "ExecutionEnvironment")
    env_hosts: ElementTree.Element = ElementTree.SubElement(env, 'HostList')
    env_locales: ElementTree.Element = ElementTree.SubElement(env, 'LocaleList')
    env_runtimes: ElementTree.Element = ElementTree.SubElement(env, 'RequiredRuntimeList')

    extension_info_list: ElementTree.Element = ElementTree.SubElement(manifest, "DispatchInfoList")

    # Manifest configure
    # Extension app
    for app in ENV_APPS:
        ElementTree.SubElement(env_hosts,
                               'Host',
                               {'Name': str(app),
                                'Version': str(ENV_APPS[app])})

    # Extension locale
    for locale in ENV_LOCALES:
        ElementTree.SubElement(env_locales,
                               'Locale',
                               {'Code': str(locale)})

    # Extension runtime
    for runtime in ENV_RUNTIMES:
        ElementTree.SubElement(env_runtimes,
                               'RequiredRuntime',
                               {"Name": str(runtime),
                                'Version': str(ENV_RUNTIMES[runtime])})

    # Extension instances
    for instance_index, instance in enumerate(EXTENSION_INSTANCES):
        instance_info: dict = EXTENSION_INSTANCES[instance]
        instance_id: str = f'{EXTENSION_ID}.{instance}'
        ElementTree.SubElement(extension_list,
                               "Extension",
                               {"Id": instance_id,
                                "Version": str(instance_info['version'])})

        manifest_extension_info = ElementTree.SubElement(extension_info_list,
                                                         "Extension",
                                                         {"Id": instance_id})

        manifest_extension_dispatch = ElementTree.SubElement(manifest_extension_info,
                                                             "DispatchInfo")

        # Instance resources
        instance_resources: dict = instance_info["resources"]
        manifest_instance_resources: ElementTree.Element = ElementTree.SubElement(manifest_extension_dispatch,
                                                                                  "Resources")
        instance_path: ElementTree.Element = ElementTree.SubElement(manifest_instance_resources,
                                                                    "MainPath")
        match instance_info['type']:
            case "common":
                instance_path.text = common_extension_processing(instance,
                                                                 instance_resources['path'])
            case "cpp":
                cpp_info: dict = instance_info['cpp']
                create_extension_config(instance_resources['path'],
                                        f'{EXTENSION_ID}.{instance}',
                                        f'{EXTENSION_ID}.{cpp_info["ui_instance"]}',
                                        cpp_info['constants'],
                                        cpp_info['ids'])
                instance_path.text = cpp_extension_processing(instance,
                                                              instance_resources['path'],
                                                              cpp_info)

            case _:
                raise Exception(f"Unknown extension type of {instance}")

        if 'script_path' in instance_resources:
            script_path: ElementTree.Element = ElementTree.SubElement(manifest_instance_resources,
                                                                      "ScriptPath")
            script_path.text = instance_resources['script_path']

        if 'command_line' in instance_resources:
            cef_params: ElementTree.Element = ElementTree.SubElement(manifest_instance_resources,
                                                                     "CEFCommandLine")

            for command in instance_resources['command_line']:
                cef_command: ElementTree.Element = ElementTree.SubElement(cef_params,
                                                                          "Parameter")
                cef_command.text = command

        # Instance lifecycle
        instance_lifecycle: dict = instance_info['lifecycle']
        manifest_instance_lifecycle: ElementTree.Element = ElementTree.SubElement(
            manifest_extension_dispatch,
            "Lifecycle")

        manifest_autovisible_bool: ElementTree.Element = ElementTree.SubElement(manifest_instance_lifecycle,
                                                                                "AutoVisible")
        manifest_autovisible_bool.text = str(instance_lifecycle['auto_visible']).lower()

        for action in instance_lifecycle['events']:
            instance_action: ElementTree.Element = ElementTree.SubElement(manifest_instance_lifecycle, str(action))
            for event in instance_lifecycle['events'][action]:
                app_event = ElementTree.SubElement(instance_action, 'Event')
                app_event.text = event

        # UI
        ui = instance_info['ui']
        manifest_ui = ElementTree.SubElement(manifest_extension_dispatch, "UI")

        ui_type = ElementTree.SubElement(manifest_ui, 'Type')
        ui_type.text = ui['type']

        ui_menu = ElementTree.SubElement(manifest_ui, 'Menu')
        ui_menu.text = instance_info['name']

        ui_geom = ElementTree.SubElement(manifest_ui, 'Geometry')
        ui_size = ElementTree.SubElement(ui_geom, "Size")

        width, height = ui['size']
        ui_width = ElementTree.SubElement(ui_size, 'Width')
        ui_width.text = str(width)

        ui_height = ElementTree.SubElement(ui_size, 'Height')
        ui_height.text = str(height)

        # Instance debug
        debug_instance = ElementTree.SubElement(debug_file, 'Extension', {"Id": instance_id})
        for app_index, app in enumerate(ENV_APPS):
            debug_port = 8100
            debug_port += instance_index * 10
            debug_port += app_index
            debug_hosts = ElementTree.SubElement(debug_instance, "HostList")
            ElementTree.SubElement(debug_hosts, 'Host', {"Name": app, "Port": str(debug_port)})


    manifest_tree = ElementTree.ElementTree(manifest)
    if DEBUG:
        debug_tree = ElementTree.ElementTree(debug_file)

        ElementTree.indent(debug_tree)
        ElementTree.indent(manifest_tree)

        debug_tree.write(os.path.join(BUILD_FOLDER, '.debug'), encoding="UTF-8", xml_declaration=True)


    manifest_tree.write(os.path.join(CSXS_PATH, 'manifest.xml'), encoding="UTF-8", xml_declaration=True)


# Base functions
def build_extension():
    # Build folder init
    if os.path.exists(BUILD_FOLDER):
        rmtree(BUILD_FOLDER)
    os.makedirs(BUILD_FOLDER)

    print("Config processing...")
    process_config()
    print("Done!")

    deploy()

def build_binary():
    if os.path.exists(BUILD_FOLDER):
        for instance in EXTENSION_INSTANCES:
            instance_info: dict = EXTENSION_INSTANCES[instance]
            instance_resources: dict = instance_info["resources"]
            if instance_info['type'] == 'cpp':
                cpp_extension_processing(instance,
                                         instance_resources['path'],
                                         instance_info['cpp'])
    else:
        build_extension()


def clean():
    with open('clean', 'r') as f:
        for fileline in f.read().splitlines():
            if not fileline.startswith("//"):
                if os.path.exists(fileline):
                    if os.path.isdir(fileline):
                        rmtree(fileline)
                    else:
                        os.remove(fileline)

def deploy():
    if DEBUG and PLATFORM_CODE == 'win':
        appdata: str = os.getenv("APPDATA")
        dst_path: str = os.path.join(appdata, "Adobe/CEP/extensions", EXTENSION_ID)
        if os.path.islink(dst_path):
            os.remove(dst_path)
        os.symlink(os.path.abspath(BUILD_FOLDER), dst_path, True)

# Cli
if __name__ == '__main__':
    argv = [arg for arg in sys.argv if not arg.startswith('--')]

    if len(argv) > 1:
        match sys.argv[1]:
            case 'clean':
                clean()
            case 'prepare':
                BUILD_BINARY = False
                clean()
                build_extension()
            case 'build':
                build_binary()
            case 'deploy':
                deploy()
            case _:
                print('Unknown command')
    else:
        clean()
        build_extension()
