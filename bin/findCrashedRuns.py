#!/usr/bin/env python3

from glob import glob
import code  # code.interact(local=dict(globals(), **locals()))
import argparse
import os
import pandas as pd
import re
from tqdm import tqdm
import xml.etree.ElementTree as ET
from tabulate import tabulate

parser = argparse.ArgumentParser(description='''Explore given folders try to understand 
    which runs generated some collision''')
parser.add_argument('--glob', '-g', required=True,
                    help='glob pointing to *collision.xml files')
parser.add_argument('--writecsv', '-w', default=False, action='store_true',
                    help='Activate this option if you want to save the csv of crashed simulations')
args = parser.parse_args()

filePattern = re.compile("platoform_(\d+)_(\d+)_([-+]?(?:\d*\.*\d+))_(\d+)_collision.xml")
#re.compile("platoform_(\d+)_(\d+)_([-+]?(?:\d*\.*\d+))_(\d+).csv")

print(f"Analysing results available in {args.glob}...")
files = glob(args.glob)

wrongs = []
for f in tqdm(files):
    #print(f)
    xml = None
    try:
        xml = ET.parse(f)
    except Exception as e:
        if (str(e) == "unclosed token: line 3, column 0"):
            continue  # means that simulation is still running
        print(e)
        continue

    if not xml.getroot().tag == 'collisions':
        raise Exception(f"WTF with {f}")

    collisions = list(xml.getroot())
    if not collisions:
        # No collisions detected, all right!
        continue
    else:
        '''form, vSstartPos, v1startPos, v2startPos = filePattern.search(f).groups()
        vSstartPos, v1startPos, v2startPos = int(vSstartPos), int(v1startPos), int(v2startPos)
        if (v1startPos >= vSstartPos):
            continue
        if (vSstartPos - v1startPos <= 6):
            continue
        '''
        facts = collisions[0].attrib
        crashTime, v1, v2 = float(
            facts['time']), facts['collider'], facts['victim']
        lane = int(facts['lane'].split('_')[-1])
        # if lane != 1:
        #    print("Wow, not a collision bug over middle lane!!!\nCheck " + f)
        wrongs.append([f, crashTime, v1.replace(
            "vtypeauto.", ""), v2.replace("vtypeauto.", ""), lane])

#code.interact(local=dict(globals(), **locals()))
df = pd.DataFrame(wrongs, columns=['file', 'crashTime', 'collider', 'victim', 'lane'])
df = df.sort_values(['crashTime'])
if (args.writecsv):
    df.to_csv("crashed.csv", index=False)
print(tabulate(df, showindex=False, headers='keys', tablefmt='simple'))
#print(df.to_csv(index=False, sep='\t'))


'''
def get_last_n_lines(file_name, N):
    # Create an empty list to keep the track of last N lines
    list_of_lines = []
    # Open file for reading in binary mode
    with open(file_name, 'rb') as read_obj:
        # Move the cursor to the end of the file
        read_obj.seek(0, os.SEEK_END)
        # Create a buffer to keep the last read line
        buffer = bytearray()
        # Get the current position of pointer i.e eof
        pointer_location = read_obj.tell()
        # Loop till pointer reaches the top of the file
        while pointer_location >= 0:
            # Move the file pointer to the location pointed by pointer_location
            read_obj.seek(pointer_location)
            # Shift pointer location by -1
            pointer_location = pointer_location - 1
            # read that byte / character
            new_byte = read_obj.read(1)
            # If the read byte is new line character then it means one line is read
            if new_byte == b'\n':
                # Save the line in list of lines
                list_of_lines.append(buffer.decode()[::-1])
                # If the size of list reaches N, then return the reversed list
                if len(list_of_lines) == N:
                    return list(reversed(list_of_lines))
                # Reinitialize the byte array to save next line
                buffer = bytearray()
            else:
                # If last read character is not eol then add it in buffer
                buffer.extend(new_byte)
        # As file is read completely, if there is still data in buffer, then its first line.
        if len(buffer) > 0:
            list_of_lines.append(buffer.decode()[::-1])
    # return the reversed list
    return list(reversed(list_of_lines))

def getLastKnownTimeValue(linesToScan):
    lines = linesToScan
    nlines = len(lines)
    for i in range(1,nlines+1):
        try:
            x=ET.fromstring(lines[-i])
            return float(x.attrib['end'])
        except:
            continue

################################################################
# Analysis of vec files to identify all different experiments
###############################################################



def uniqueExps(df):
    # Indentifiy the unique Experiment tuples... pandas is so verbose! :(
    uniqueExps = df.groupby(expTuple).size().reset_index()
    uniqueExps = uniqueExps[expTuple].set_index(expTuple)
    uniqueExps = uniqueExps.index.to_list()
    return uniqueExps

filePattern = re.compile("([a-zA-Z]+)_(\d+)_(\d+)_(0.\d+|\d+)_\d+.")
controllerPattern = re.compile("Ring([A-Z]+)NoGui")

vecs = [f.split('/')[-1] for f in files if f.endswith('.vec')]

data = []
for f in vecs:
    #code.interact(local=dict(globals(), **locals()))
    try:
        sc, ncars, ps, pr = filePattern.search(f).groups()
        sc = controllerPattern.search(sc).groups()[0]
        data.append([sc, int(ncars), int(ps), float(pr)])
    except AttributeError:
        print("occhio! "+f)
        continue
    

df = pd.DataFrame(data, columns = expTuple)


ues = uniqueExps(df)

linelen = 60
for sc in sorted(df.sController.unique()):
    print("="*linelen)
    print(f" {sc}")
    tmp = df.query("sController == @sc")
    nc_vals = sorted(list(tmp.nCars.unique()))
    ps_vals = sorted(list(tmp.platoonSize.unique()))
    pr_vals = sorted(list(tmp.pRate.unique()))

    depth = 1
    for l in [nc_vals, ps_vals, pr_vals]:
        if len(l) > 1:
            print("\t"*depth, l)
        depth+=1
print("="*linelen)

'''
