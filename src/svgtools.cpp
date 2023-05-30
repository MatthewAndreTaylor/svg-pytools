/**
 * Copyright (c) 2023 Matthew Andre Taylor
 */
#include <Python.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

typedef enum {
  RECTANGLE,
  CIRCLE,
  ELLIPSE,
  LINE,
  POLYLINE,
  POLYGON,
  PATH,
  CONTAINER_OPEN,
  CONTAINER_CLOSE
} PrimitiveType;

typedef struct {
  std::string key;
  std::string value;
} Pair;

typedef struct Node {
  PrimitiveType type;
  std::string dString;
  std::vector<Pair> attr;
} PathNode;

std::regex primitiveRegex(".*<(rect|circle|ellipse|line|polyline|polygon|path)",
                          std::regex_constants::icase);
std::regex attrRegex(R"(([a-zA-Z][a-zA-Z0-9_-]*)=\"([^\"]*)\")",
                     std::regex_constants::icase);
std::regex closingtagRegex(R"(</\w+>)", std::regex_constants::icase);

PathNode ParsePrimitive(std::string &input) {
  std::stringstream ss;
  std::smatch match;

  PathNode p;
  if (std::regex_search(input, match, primitiveRegex)) {
    std::string primitiveType = match.str(1);
    if (primitiveType == "rect") {
      p.type = RECTANGLE;
    }
    else if (primitiveType == "circle") {
      p.type = CIRCLE;
    }
    else if (primitiveType == "ellipse") {
      p.type = ELLIPSE;
    }
    else if (primitiveType == "line") {
      p.type = LINE;
    }
    else if (primitiveType == "polyline") {
      p.type = POLYLINE;
    }
    else if (primitiveType == "polygon") {
      p.type = POLYGON;
    }
    else if (primitiveType == "path") {
      p.type = PATH;
    }
  }
  else if (std::regex_search(input, closingtagRegex)) {
    p.type = CONTAINER_CLOSE;
    return p;
  } else {
    p.type = CONTAINER_OPEN;
  }

  float x = 0, y = 0, width = 0, height = 0;
  std::string::size_type pos = (match.size() > 0) ? match[0].length() : 0;

  std::string attributes = input.substr(pos);

  std::sregex_iterator it(attributes.begin(), attributes.end(), attrRegex);
  std::sregex_iterator end;

  while (it != end) {
    std::smatch match = *it;
    std::string attrName = match.str(1);
    std::string attrValue = match.str(2);

    switch (p.type)
    {
      case RECTANGLE:
        if (attrName == "x") {
          x = std::stof(attrValue);
        } else if (attrName == "y") {
          y = std::stof(attrValue);
        } else if (attrName == "width") {
          width = std::stof(attrValue);
        } else if (attrName == "height") {
          height = std::stof(attrValue);
        } else {
          p.attr.push_back({std::move(attrName), std::move(attrValue)});
        }
        break;
      
      case CIRCLE:
        if (attrName == "cx") {
          x = std::stof(attrValue);
        } else if (attrName == "cy") {
          y = std::stof(attrValue);
        } else if (attrName == "r") {
          width = std::stof(attrValue);
        } else {
          p.attr.push_back({std::move(attrName), std::move(attrValue)});
        }
        break;

      case ELLIPSE:
        if (attrName == "cx") {
          x = std::stof(attrValue);
        } else if (attrName == "cy") {
          y = std::stof(attrValue);
        } else if (attrName == "rx") {
          width = std::stof(attrValue);
        } else if (attrName == "ry") {
          height = std::stof(attrValue);
        } else {
          p.attr.push_back({std::move(attrName), std::move(attrValue)});
        }
        break;

      case LINE:
        if (attrName == "x1") {
          x = std::stof(attrValue);
        } else if (attrName == "y1") {
          y = std::stof(attrValue);
        } else if (attrName == "x2") {
          width = std::stof(attrValue);
        } else if (attrName == "y2") {
          height = std::stof(attrValue);
        } else {
          p.attr.push_back({std::move(attrName), std::move(attrValue)});
        }
        break;

      case POLYLINE:
        if (attrName == "points") {
          std::string point =
              std::regex_replace(attrValue, std::regex(" "), " L");
          ss << "M" << point;
        } else {
          p.attr.push_back({std::move(attrName), std::move(attrValue)});
        }
        break;

      case POLYGON:
        if (attrName == "points") {
          std::string point =
              std::regex_replace(attrValue, std::regex(" "), " L");
          ss << "M" << point << " Z";
        } else {
          p.attr.push_back({std::move(attrName), std::move(attrValue)});
        }
        break;

      case PATH:
        if (attrName == "d") {
          ss << attrValue;
        } else {
          p.attr.push_back({std::move(attrName), std::move(attrValue)});
        }
        break;
      
      default:
        p.attr.push_back({std::move(attrName), std::move(attrValue)});
    }
    ++it;
  }
  if (p.type == RECTANGLE) {
    ss << "M" << x << "," << y << " H" << x + width << " V" << y + height
       << " H" << x << " Z";
  } else if (p.type == CIRCLE) {
    ss << "M" << x - width << "," << y << " A" << width << "," << width
       << " 0 0,0 " << x + width << "," << y << " A" << width << "," << width
       << " 0 0,0 " << x - width << "," << y << " Z";
  } else if (p.type == ELLIPSE) {
    ss << "M" << x - width << "," << y << " A" << width << "," << height
       << " 0 0,0 " << x + width << "," << y << " A" << width << "," << height
       << " 0 0,0 " << x - width << "," << y << " Z";
  } else if (p.type == LINE) {
    ss << "M" << x << "," << y << " L" << width << "," << height;
  }
  p.dString = ss.str();
  return p;
}

std::vector<std::string> splitSVGNodes(const std::string &svgContent) {
  std::vector<std::string> nodes;
  std::istringstream svgStream(svgContent);
  std::string line;
  std::string currentNode;

  while (std::getline(svgStream, line)) {
    currentNode += line;

    // Check if the current node is complete (ends with ">") or a closing tag
    if (currentNode.find(">") != std::string::npos ||
        std::regex_search(currentNode, closingtagRegex)) {
      nodes.push_back(currentNode);
      currentNode.clear();
    }
  }

  return nodes;
}

PyDoc_STRVAR(svgtools_parse_doc, "Parse SVG content into a dictionary.");
static PyObject *svgtools_parse(PyObject *self, PyObject *args) {
  const char *svgContent;

  if (!PyArg_ParseTuple(args, "s", &svgContent)) {
    return NULL;
  }
  
  std::string content(svgContent);

  // Call the splitSVGNodes and ParsePrimitive functions to process the SVG
  std::vector<std::string> nodes = splitSVGNodes(content);

  PyObject *childKey = PyUnicode_FromString("container");

  PyObject *dict = PyDict_New();
  Py_INCREF(dict);

  PyObject *currDict = dict;
  std::vector<PyObject*> containerStack;
  containerStack.push_back(dict);

  for (size_t i = 0; i < nodes.size(); i++) {
    std::string &node = nodes[i];
    PathNode parsedNode = ParsePrimitive(node);

    if (parsedNode.type == CONTAINER_OPEN) {
      PyObject *d = PyDict_New();
      Py_INCREF(d);

      for (size_t j = 0; j < parsedNode.attr.size(); j++) {
        const std::string &key = parsedNode.attr[j].key;
        const std::string &attrVal = parsedNode.attr[j].value;
        PyObject *val = PyUnicode_FromString(attrVal.c_str());
        PyDict_SetItemString(d, key.c_str(), val);
      }

      PyObject* children = PyDict_SetDefault(currDict, childKey, PyList_New(0));
      PyList_Append(children, d);
      currDict = d;
    } else if (parsedNode.type == CONTAINER_CLOSE) {
      containerStack.pop_back();
      currDict = containerStack.back();
    } else {
      if (!parsedNode.dString.empty()) {
        PyObject *dom = PyDict_New();
        PyObject *dStringValue = PyUnicode_FromString(parsedNode.dString.c_str());
        PyDict_SetItemString(dom, "d", dStringValue);

        for (size_t j = 0; j < parsedNode.attr.size(); j++) {
          const std::string &key = parsedNode.attr[j].key;
          const std::string &attrVal = parsedNode.attr[j].value;
          PyObject *val = PyUnicode_FromString(attrVal.c_str());
          PyDict_SetItemString(dom, key.c_str(), val);
        }

        PyObject* children = PyDict_SetDefault(currDict, childKey, PyList_New(0));
        PyList_Append(children, dom);
      }
    }
  }

  return containerStack.front();
}

PyDoc_STRVAR(svgtools_to_path_str_doc, "Return a new svg where each primitive is converted to a path.");
static PyObject *svgtools_to_path_str(PyObject *self, PyObject *args) {
  const char *svgContent;

  if (!PyArg_ParseTuple(args, "s", &svgContent)) {
    return NULL;
  }
  
  std::string content(svgContent);

  // Call the splitSVGNodes and ParsePrimitive functions to process the SVG
  std::vector<std::string> svgNodes = splitSVGNodes(content);

  std::stringstream result;
  result << "<svg xmlns=\"http://www.w3.org/2000/svg\">" << std::endl;
  for (std::string& node : svgNodes) {
        PathNode p = ParsePrimitive(node);
        if (!p.dString.empty()) {
            result << "<path d=\"" << p.dString << "\"";
            for (Pair pair : p.attr) {
                result << " " << pair.key << "=\"" << pair.value << "\"";
            }
            result << "/>\n";
        }
    }
    result << "</svg>";
    return PyUnicode_FromString(result.str().c_str());
}

static PyMethodDef SvgParserMethods[] = {
    {"parse", svgtools_parse, METH_VARARGS, svgtools_parse_doc},
    {"to_path_str", svgtools_to_path_str, METH_VARARGS, svgtools_to_path_str_doc},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef svgparsermodule = {PyModuleDef_HEAD_INIT, "svgtools",
                                             NULL, -1, SvgParserMethods};

PyMODINIT_FUNC PyInit_svgtools() { return PyModule_Create(&svgparsermodule); }