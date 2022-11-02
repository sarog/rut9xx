#!/usr/bin/env python3

import mysql.connector
import glob
import logging
import boto3
from botocore.exceptions import ClientError
import os

MYSQL_USER = "root"
MYSQL_SERVER = "demorut.teltonika.lt"
MYSQL_DATABASE = "rut_fota"

FILE_LOCATION = "./bin/targets*"
BUCKET_PATH = "test/firmware"
S3_BUCKET = "rut-fota"
ROOT_S3_PATH = "test/firmware/"

# AWS API credentials are read from environmental variables:
# AWS_ACCESS_KEY_ID and
# AWS_SECRET_ACCESS_KEY

def upload_file(file_name, bucket, object_name):
    """Upload a file to an S3 bucket

    :param file_name: File to upload
    :param bucket: Bucket to upload to
    :return: True if file was uploaded, else False
    """

    # Upload the file
    s3_client = boto3.client('s3')
    try:
        response = s3_client.upload_file(file_name, bucket, object_name)
    except ClientError as e:
        logging.error(e)
        return False
    return True


def create_connection():
    connection = mysql.connector.connect(
        host = MYSQL_SERVER,
        user = MYSQL_USER,
        password = os.getenv('MYSQL_PASS'),
        database = MYSQL_DATABASE
    )
    cursor = connection.cursor()

    return connection, cursor

def get_relation_ids_by_router(router):
    print(router)
    if router == "TRB1":
        return [10]
    elif router == "RUT2M":
        return [49, 53]
    elif router == "TCR1":
        return [41]
    elif router == "RUT36X":
        return [17]
    elif router == "RUT9":
        return [47, 48, 3]
    elif router == "TRB2":
        return [7]
    elif router == "RUTX":
        return [1]
    elif router == "RUT9M":
        return [50, 51]
    elif router == "RUT30X":
        return [35]
    elif router == "RUT2":
        return [45]
    else:
        return None


def get_router_bucket(filename_root):
    if filename_root == "RUT30X":
        return "RUT300"
    elif filename_root == "RUT36X":
        return "RUT360"
    elif filename_root == "TRB1":
        return "TRB"
    else:
        return filename_root


def construct_relation_query(cursor, updated_index, router):
    # safe guard if no index is provided, that way incorrect queries are not sent
    if not updated_index:
        return
    ids = get_relation_ids_by_router(router)
    if not ids:
        return
    for id in ids:
        query = f"UPDATE `rut_fota`.`suggested_firmwares` SET `file_id` = '{updated_index}' WHERE (`id` = '{id}');"
        cursor.execute(query)


def insert_file_query(cursor, query):
    cursor.execute(query)
    file_id = cursor.lastrowid

    return file_id


def construct_query(file_path):
    filename_without_ext = get_file_name(file_path)
    filename_root = filename_without_ext.split("_")[0]
    router_bucket = get_router_bucket(filename_root)
    amazon_path = f"{BUCKET_PATH}/{router_bucket}/{filename_without_ext}.bin"
    file_size = os.path.getsize(file_path)
    query = f"INSERT INTO `rut_fota`.`files` (`name`, `path`, `files_type_id`, `size`) VALUES ('{filename_without_ext}', '{amazon_path}', '1', '{file_size}')"
    return query, filename_root


def get_file_name(file_path):
    filename, _ = os.path.splitext(file_path)
    filename_without_ext = os.path.basename(filename)
    return filename_without_ext


def upload_firmware(file_path):
    filename_without_ext = get_file_name(file_path)
    filename_root = filename_without_ext.split("_")[0]
    router_bucket = get_router_bucket(filename_root)
    s3_file_path = f'{ROOT_S3_PATH}{router_bucket}/{filename_without_ext}.bin'
    upload_file(file_path, S3_BUCKET, s3_file_path)


def main():
    connection, cursor = create_connection()
    for file in glob.glob(f'{FILE_LOCATION}/**/*[0-9]_WEBUI*.bin', recursive=True):
        print(file)
        upload_firmware(file)
        query, router = construct_query(file)
        print("Query " + query)
        updated_index = insert_file_query(cursor, query)
        construct_relation_query(cursor, updated_index, router)
        connection.commit()


if __name__ == "__main__":
    main()
