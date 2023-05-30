import svgtools
import json

content = '''
<svg xmlns="http://www.w3.org/2000/svg" width="400" height="400">
  <rect x="50" y="50" width="100" height="50" fill="blue" />
  <circle cx="200" cy="100" r="50" fill="red" />
  <ellipse cx="350" cy="75" rx="50" ry="25" fill="green" />
  <line x1="50" y1="200" x2="150" y2="300" stroke="orange" />
  <polyline points="200,200 250,250 300,200 350,250" fill="none" stroke="purple" />
  <polygon points="350,200 400,250 400,150" fill="yellow" />
  <path d="M50,350 L100,350 Q125,375 150,350 T200,350" fill="none" stroke="black"/>

  <rect x="10" y="10" height="100" width="100"
        style="stroke:#ff0000; fill: #0000ff"/>
        <path d="M50,350 L100,350 Q125,375 150,350 T200,350" fill="none" stroke="black"/><polygon points="350,200 400,250 400,150" fill="yellow" />


  <circle cx="200" cy="100" r="50" fill="red"></circle>

  <polygon points="350,200 400,250 400,150" fill="yellow" />
</svg>
'''

nodes = svgtools.parse(content)

# Prints the formatted dictionary
print(json.dumps(nodes,sort_keys=True, indent=4))

test1 = '''
<svg xmlns="http://www.w3.org/2000/svg" width="200" height="200">
  <rect x="10" y="10" width="180" height="180" fill="lightgray" />
  
  <g transform="translate(100,100)">
    <circle cx="0" cy="0" r="80" fill="blue" />
    
    <g transform="translate(-40,-40)">
      <rect x="0" y="0" width="80" height="80" fill="red" />
      <text x="40" y="60" text-anchor="middle" fill="white">Nested</text>
    </g>
  </g>
</svg>
'''

print(json.dumps(svgtools.parse(test1),sort_keys=True, indent=4))

s = svgtools.to_path_str(content)

print(s)