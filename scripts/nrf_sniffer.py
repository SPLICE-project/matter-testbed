#!/usr/bin/env python3
from nrf802154_sniffer import Nrf802154Sniffer
import pyshark
import queue as Queue
import tempfile
from pymongo import MongoClient
from os import remove

class RealTimeNrfSniffer(Nrf802154Sniffer):
    def parse_packet(self, parsed_packet):
        data = {}
        for layer in parsed_packet.layers:
            if layer.layer_name:  # Check if the layer has a name
                current_layer = {}
                data[layer.layer_name] = current_layer
                for field_name in layer.field_names:
                    field_value = getattr(layer, field_name, None)
                    if field_value:
                        current_layer[field_name] = field_value
        return data

    def fifo_writer(self, fifo, queue):

        client = MongoClient("mongodb://managua.kiewit.dartmouth.edu:27017/")
        db = client['matter_packet_capture']
        collection = db['thread']


        with open(fifo, 'wb', 0) as fh:
            fh.write(self.pcap_header())
            fh.flush()

            while self.running.is_set():
                try:
                    packet = queue.get(block=True, timeout=1)
                    try:
                        with tempfile.NamedTemporaryFile(delete=False) as temp_file:
                            temp_file.write(self.pcap_header())
                            temp_file.write(packet)
                            temp_file_name = temp_file.name

                        capture = pyshark.FileCapture(temp_file_name)

                        for unparsed_packet in capture:
                            try:
                                output = self.parse_packet(unparsed_packet)
                                collection.insert_one(output)
                                print("****\n", output, "\n****\n")
                            except Exception as e:
                                print("Unable to add packet to database.\n", e)

                        remove(temp_file_name)

                    except IOError:
                        pass

                except Queue.Empty:
                    pass


if __name__ == "__main__":
    sniffer = RealTimeNrfSniffer()
    sniffer.extcap_capture(fifo="/tmp/file.pcap", dev="/dev/ttyACM0", channel=25)

