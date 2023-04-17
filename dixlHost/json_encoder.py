"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from enum import Enum
from json import JSONEncoder

class JSONEncoderCustom(JSONEncoder):
    def default(self, obj):
        if hasattr(obj, 'to_json'):
            return obj.to_json()
        
        if isinstance(obj, Enum):
            return obj.name
        
        raise TypeError(f'Object of type {obj.__class__.__name__} is not JSON serializable')
    