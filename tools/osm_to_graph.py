#!/usr/bin/python3
"""
Read an OSM's XML file from standart input and outputs roads nodes.
More about OSM's XML: https://wiki.openstreetmap.org/wiki/OSM_XML.
"""
from sys import stdin

import xml.etree.ElementTree as ET


xml = ET.iterparse(stdin, events=['start'])


last_node = None
_, root = next(xml)

for event, element in xml:
    if element.tag == 'way':
        last_node = None
    elif element.tag == 'nd':
        if last_node != None:
            print(last_node, element.attrib['ref'])

        last_node = element.attrib['ref']

    root.clear()
