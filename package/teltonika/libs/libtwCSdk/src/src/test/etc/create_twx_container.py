#
# TO USE (TL;DR):
#
#    * Stop any running thingworx container. Note: Any container or image with
#      the same name as this script will create will be deleted, resulting in
#      data loss!
#    * Run the script. There are default arguments provided. If you're okay with
#      those, just run:
#
#         $ python3 create_twx_container.py
#
#      Otherwise, get a listing of the command line arguments with:
#
#         $ python3 create_twx_container.py -h
#
# DESCRIPTION:
#
# Create and run a Thingworx Docker image and container suitable for testing
# the CDSK. An application key is installed into the container whose value
# is in the testconfig.json file specified in the arguments.
#
# This is to simplify docker container creation for those of us who use a
# persistent container for development instead of letting the test script spin
# one up each time. (I got tired of having to type all those port numbers on
# the docker run command).
#
# If a docker image or container exist with the specified names, they will be
# replaced by the freshly built image and container.
#
# If a docker container with the specified name is running when this application
# is executed, the application will abort.
#
# To test SCM, utilities needs to be manually installed on the container.
#
# Requires Python 3+.
#

import argparse
import json
import os
import shlex
import subprocess
import time


def parse_command_line() -> argparse.Namespace:
    """Parse command line arguments.

    Returns:
        command line arguments
    """
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '-b', '--base-image',
        default='thingworx-foundation-h2',
        help='Base thingworx Docker image to use'
    )
    parser.add_argument(
        '-c', '--container-name',
        default='thingworx-{version}',
        help='Name of created container'
    )
    parser.add_argument(
        '--dockerfile',
        default='docker',
        help='Directory containing Dockerfile'
    )
    parser.add_argument(
        '-i', '--image-name',
        default='thingworx-{version}',
        help='Name of created image'
    )
    parser.add_argument(
        '--keep-image',
        action='store_true',
        help='Do not replace current image'
    )
    parser.add_argument(
        '--key-file',
        default='IntegrationTestKey.xml',
        help='path to integration test key file'
    )
    parser.add_argument(
        '--test-config',
        default='testconfig.json',
        help='path to testconfig file'
    )
    parser.add_argument(
        '-v', '--thingworx-version',
        default='8.1.0-b52',
        help='Base thingworx version to use'
    )

    args = parser.parse_args()
    args.image_name = args.image_name.format(
        version=args.thingworx_version
    )
    args.container_name = args.container_name.format(
        version=args.thingworx_version
    )
    args.dockerfile = os.path.abspath(
        os.path.expanduser(args.dockerfile)
    )
    args.key_file = os.path.abspath(
        os.path.expanduser(args.key_file)
    )
    args.test_config = os.path.abspath(
        os.path.expanduser(args.test_config)
    )

    return args


def run(cmd: str, shell: bool = False) -> int:
    """Run shell command.

    Args:
        cmd:  command to execute.
        shell: run command in shell.

    Returns:
        return code (0 => success)
    """
    print('>>> {cmd}'.format(cmd=cmd))

    if not shell:
        cmd = shlex.split(cmd)

    res = subprocess.run(cmd, shell=shell)

    return res.returncode


def container_running(name: str) -> bool:
    """Is container `name` running?

    Args:
        name: name of container.

    Returns:
        container is running.
    """
    # Look for the exact container name.
    cmd = 'docker ps | grep "\s{name}\s"'.format(name=name)
    return run(cmd, shell=True) == 0


def remove_container(name: str) -> int:
    """Remove container `name`.

    Args:
        name: name of container.

    Returns:
        command return code (0 => success)
    """
    cmd = 'docker ps -a | grep "\s{name}\s"'.format(name=name)
    if run(cmd, shell=True) == 0:
        # Container named `name` found. Remove.
        cmd = 'docker rm {name}'.format(name=name)
        res = run(cmd)
    else:
        # Container named `name` not found.
        res = 0

    return res


def remove_image(name: str) -> int:
    """Remove image `name`.

    Args:
        name: name of image.

    Returns:
        command return code (0 => success)
    """
    cmd = 'docker images | grep "\s{name}\s"'.format(name=name)
    if run(cmd, shell=True) == 0:
        # Image named `name` found. Remove.
        cmd = 'docker rmi {name}'.format(name=name)
        res = run(cmd)
    else:
        # Image named `name` not found.
        res = 0

    return res


def copy_license(version: str) -> int:
    """Copy `license.bin` for requested platform version.

    Args:
        version: Base thingworx version.

    Returns:

    """
    # Assuming semantic versioning with build (MAJOR.MINOR.PATCH-BUILD),
    # licenses are only changed on MINOR releases, so we only want the MAJOR
    # and MINOR values.
    version_elements = version.split('.')
    cmd = 'cp docker/twx-licenses/{major}.{minor}/license.bin .'.format(
        major=version_elements[0],
        minor=version_elements[1]
    )
    return run(cmd)


def create_image(
    name: str,
    dockerfile: str,
    base_image: str,
    base_version: str
) -> int:
    """Create image using specified Dockerfile.

    Args:
        name: name of created image.
        dockerfile: path to Dockerfile.
        base_image: base image name.
        base_version: base image version.

    Returns:
        command return code (0 => success)
    """
    cmd = (
        'docker build '
        '--build-arg base={base_image} '
        '--build-arg version={base_version} '
        '-t {name} '
        '{dockerfile}'.format(
            base_image=base_image,
            base_version=base_version,
            name=name,
            dockerfile=dockerfile
        )
    )
    return run(cmd)


def create_container(name: str, image_name: str) -> int:
    """Create container `name` from image `image_name`

    Args:
        name:
        image_name:

    Returns:
        command return code (0 => success)
    """
    cmd = (
        'docker run '
        '-d '
        '-p 4443:4443 '
        '-p 4444:4444 '
        '-p 4445:4445 '
        '-p 8080:8080 '
        '-p 8443:8443 '
        '--name {name} '
        '{image_name}'.format(name=name, image_name=image_name)
    )
    return run(cmd)


def extract_from_config(config: str, field: str) -> str:
    """Extract field from config file.

    Args:
        config: path to config file.
        field: field to extract from config.

    Returns:
        field value.
    """
    with open(config, 'r') as f:
        settings = json.load(f)

    settings = settings.get('rows')
    return settings[0].get(field)


def install_api_key(testconfig: str, keyfile: str) -> int:
    """Install api key from testconfig file.

    Args:
        testconfig: path to testconfig.json.
        keyfile: path to key file.

    Returns:
        command return code (0 => success)
    """
    cmd = extract_from_config(testconfig, 'bootstraphttpuploadexecutable')
    cmd = cmd % (keyfile, 'localhost', 8443)
    return run(cmd)


def main() -> int:
    args = parse_command_line()

    # Verify initial conditions.
    if container_running(args.container_name):
        print(
            'Docker container named {args.container_name} already running. '
            'You must stop it before running this application.'.format(
                args=args
            )
        )
        return -1

    # Create thingworx container.
    res = remove_container(args.container_name)
    if res:
        print(
            'Unable to remove container, {args.container_name}. Aborting. '
            'code={res}'.format(args=args, res=res)
        )
        return res

    if not args.keep_image:
        res = remove_image(args.image_name)
        if res:
            print(
                'Unable to remove image, {args.image_name}. Aborting. '
                'code={res}'.format(args=args, res=res)
            )
            return res

        res = copy_license(args.thingworx_version)
        if res:
            print('Unable to copy license.bin. Aborting.')
            return res

        res = create_image(
            args.image_name,
            args.dockerfile,
            args.base_image,
            args.thingworx_version
        )
        if res:
            print(
                'Unable to create image, {args.image_name}. Aborting. '
                'code={res}'.format(args=args, res=res)
            )
            return res

    res = create_container(args.container_name, args.image_name)
    if res:
        print(
            'Unable to create container, {args.container_name}. Aborting. '
            'code={res}'.format(args=args, res=res)
        )
        return res

    # Wait for container to spin up.
    time.sleep(5)

    res = install_api_key(args.test_config, args.key_file)
    if res:
        print(
            'Unable to install api key. You will have to do it manually. '
            'code={res}'.format(res=res)
        )

    return 0


if __name__ == '__main__':
    main()
