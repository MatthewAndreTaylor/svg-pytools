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
  CONTAINER
} PrimitiveType;

typedef struct {
  PrimitiveType type;
  std::string dString;
  std::vector<std::tuple<std::string, std::string>> attr;
} PathNode;

std::regex rectRegex(".*<rect", std::regex_constants::icase);
std::regex circleRegex(".*<circle", std::regex_constants::icase);
std::regex ellipseRegex(".*<ellipse", std::regex_constants::icase);
std::regex lineRegex(".*<line", std::regex_constants::icase);
std::regex polylineRegex(".*<polyline", std::regex_constants::icase);
std::regex polygonRegex(".*<polygon", std::regex_constants::icase);
std::regex pathRegex(".*<path", std::regex_constants::icase);

std::regex attrRegex(R"(([a-zA-Z][a-zA-Z0-9_-]*)=\"([^\"]*)\")",
                     std::regex_constants::icase);

std::regex closingtagRegex(R"(</\w+>)", std::regex_constants::icase);

PathNode ParsePrimitive(std::string &input) {
  std::stringstream ss;
  std::smatch match;

  PathNode p;
  if (std::regex_search(input, match, rectRegex)) {
    p.type = RECTANGLE;
  } else if (std::regex_search(input, match, circleRegex)) {
    p.type = CIRCLE;
  } else if (std::regex_search(input, match, ellipseRegex)) {
    p.type = ELLIPSE;
  } else if (std::regex_search(input, match, lineRegex)) {
    p.type = LINE;
  } else if (std::regex_search(input, match, polylineRegex)) {
    p.type = POLYLINE;
  } else if (std::regex_search(input, match, polygonRegex)) {
    p.type = POLYGON;
  } else if (std::regex_search(input, match, pathRegex)) {
    p.type = PATH;
  } else {
    p.type = CONTAINER;
    return p;
  }

  float x = 0, y = 0, width = 0, height = 0;
  std::string::size_type pos = (match.size() > 0) ? match[0].length() : 0;

  std::string attr = input.substr(pos, input.length() - 2);

  std::sregex_iterator it(attr.begin(), attr.end(), attrRegex);
  std::sregex_iterator end;

  while (it != end) {
    std::smatch match = *it;
    std::string attrName = match.str(1);
    std::string attrValue = match.str(2);

    if (p.type == RECTANGLE) {
      if (attrName == "x") {
        x = std::stof(attrValue);
      } else if (attrName == "y") {
        y = std::stof(attrValue);
      } else if (attrName == "width") {
        width = std::stof(attrValue);
      } else if (attrName == "height") {
        height = std::stof(attrValue);
      } else {
        p.attr.push_back(std::make_tuple(attrName, attrValue));
      }
    } else if (p.type == CIRCLE) {
      if (attrName == "cx") {
        x = std::stof(attrValue);
      } else if (attrName == "cy") {
        y = std::stof(attrValue);
      } else if (attrName == "r") {
        width = std::stof(attrValue);
      } else {
        p.attr.push_back(std::make_tuple(attrName, attrValue));
      }
    } else if (p.type == ELLIPSE) {
      if (attrName == "cx") {
        x = std::stof(attrValue);
      } else if (attrName == "cy") {
        y = std::stof(attrValue);
      } else if (attrName == "rx") {
        width = std::stof(attrValue);
      } else if (attrName == "ry") {
        height = std::stof(attrValue);
      } else {
        p.attr.push_back(std::make_tuple(attrName, attrValue));
      }
    } else if (p.type == LINE) {
      if (attrName == "x1") {
        x = std::stof(attrValue);
      } else if (attrName == "y1") {
        y = std::stof(attrValue);
      } else if (attrName == "x2") {
        width = std::stof(attrValue);
      } else if (attrName == "y2") {
        height = std::stof(attrValue);
      } else {
        p.attr.push_back(std::make_tuple(attrName, attrValue));
      }
    } else if (p.type == POLYLINE) {
      if (attrName == "points") {
        std::string point =
            std::regex_replace(attrValue, std::regex(" "), " L");
        ss << "M" << point;
      } else {
        p.attr.push_back(std::make_tuple(attrName, attrValue));
      }
    } else if (p.type == POLYGON) {
      if (attrName == "points") {
        std::string point =
            std::regex_replace(attrValue, std::regex(" "), " L");
        ss << "M" << point << " Z";
      } else {
        p.attr.push_back(std::make_tuple(attrName, attrValue));
      }
    } else if (p.type == PATH) {
      if (attrName == "d") {
        ss << attrValue;
      } else {
        p.attr.push_back(std::make_tuple(attrName, attrValue));
      }
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

  // Create a Python list to store the processed nodes
  PyObject *nodeList = PyList_New(nodes.size());
  Py_INCREF(nodeList);

  // Iterate over the nodes and parse each one using the ParsePrimitive function
  for (size_t i = 0; i < nodes.size(); i++) {
    std::string &node = nodes[i];
    PathNode parsedNode = ParsePrimitive(node);

    // Create a Python dictionary to represent the parsed node
    PyObject *dict = PyDict_New();
    Py_INCREF(dict);

    // Set the 'dString' key in the dictionary to the dString value
    PyObject *dStringValue = PyUnicode_FromString(parsedNode.dString.c_str());
    PyDict_SetItemString(dict, "d", dStringValue);

    // Iterate over the attribute strings and append them to the attrList
    for (size_t j = 0; j < parsedNode.attr.size(); j++) {
      const std::string &key = std::get<0>(parsedNode.attr[j]);
      const std::string &attrVal = std::get<1>(parsedNode.attr[j]);
      PyObject *val = PyUnicode_FromString(attrVal.c_str());
      PyDict_SetItemString(dict, key.c_str(), val);
    }

    // Set the dictionary as an item in the nodeList
    PyList_SetItem(nodeList, i, dict);
  }

  // Return the Python list containing the processed nodes
  return nodeList;
}

static PyMethodDef SvgParserMethods[] = {
    {"parse", svgtools_parse, METH_VARARGS, svgtools_parse_doc},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef svgparsermodule = {PyModuleDef_HEAD_INIT, "svgtools",
                                             NULL, -1, SvgParserMethods};

PyMODINIT_FUNC PyInit_svgtools() { return PyModule_Create(&svgparsermodule); }