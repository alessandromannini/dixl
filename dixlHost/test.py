from point import Point
from point_ref import PointRef, PointPosition
from route import Route
from track_circuit import TrackCircuit
from track_circuit_ref import TrackCircuitRef
from layout import Layout
import json
from  json_encoder import JSONEncoderCustom

# Nodes
tc = TrackCircuit("TC1", b"\x7a\x7a\xc0\xa8\xad\x50")
pt = Point("PT1", b"\x7a\x7a\xc0\xa8\xad\x5A")

# Routes
route1 = Route(1,   [   TrackCircuitRef(tc),
                        PointRef(pt, PointPosition.DIVERGING)
                    ])

route2 = Route(2,   [   TrackCircuitRef(tc),
                        PointRef(pt, PointPosition.STRAIGHT)
                    ])

# Layout
layout = Layout("main","test scenarios")

# Adding nodes
layout.NodeAdd(tc)
layout.NodeAdd(pt)

# Adding routes
layout.RouteAdd(route1)
layout.RouteAdd(route2)

# JSON output test
print(json.dumps(tc, cls=JSONEncoderCustom, indent=4))
print(json.dumps(pt, cls=JSONEncoderCustom, indent=4))
print(json.dumps(route1, cls=JSONEncoderCustom, indent=4))
print(json.dumps(route2, cls=JSONEncoderCustom, indent=4))
print(json.dumps(layout, cls=JSONEncoderCustom, indent=4))

# Write to disk
# layout.WriteToFile()

# Read from disk
ll = Layout.ReadFromFile(filename="main.dixl")
