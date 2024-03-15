#!/usr/bin/env python3

import pymongo
import json
import sys

# Establish a connection to the MongoDB server
client = pymongo.MongoClient("mongodb://managua.kiewit.dartmouth.edu:27017/")

# Specify the database and collection
db = client["matter_packet_capture"]
collection = db["ethernet"]

def insert_packet_to_mongodb(packet_data):
    try:
        collection.insert_one(packet_data)
        print("Packet inserted successfully!")
    except Exception as e:
        print(f"An error occurred: {e}")

def main():
    for line in sys.stdin:
        try:
            packet_data = json.loads(line)
            insert_packet_to_mongodb(packet_data)
        except json.JSONDecodeError:
            print("Invalid JSON received. Skipping.")

if __name__ == "__main__":
    main()
