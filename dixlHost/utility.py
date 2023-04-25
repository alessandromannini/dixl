"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
def filename_sanitize(filename: str) -> str:
    validchars = "-_.() "
    out: str = ""
    for c in filename:
      if str.isalpha(c) or str.isdigit(c) or (c in validchars):
        out += c
      else:
        out += "_"
    return out

def str2MAC(v: str) -> bytes:
    slices = str.split(v,":")
    if len(slices) < 6:
        slices = str.split(v, "-")
        if len(slices) < 6:
           if len(v) == 12:
              slices = [v[i:i+2] for i in range(0, len(v), 2)]
           else:
              raise ValueError("Invalid MAC value {}".format(v))
    
    values = [int(slice, 16) for slice in slices]

    return bytes(values)

def MAC2str(MAC: bytes) -> str:
   return ":".join([f'{b:02x}' for b in MAC])

def IP2str(IP: bytearray) -> str:
   return ".".join([f'{b}' for b in IP])

def str2IP(v: str) -> bytes:
    slices = str.split(v,".")    
    values = [int(slice) for slice in slices]
    return bytes(values)


