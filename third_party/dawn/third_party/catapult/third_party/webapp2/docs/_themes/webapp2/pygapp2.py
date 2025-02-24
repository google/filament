from __future__ import absolute_import
from pygments.style import Style
from pygments.token import *


class pygapp2(Style):
    background_color = "#FAFAFA"
    default_style = ""

    styles = {
        Whitespace:                "nobold noitalic #FFF",
        Comment:                   "nobold noitalic #800",
        Comment.Preproc:           "nobold noitalic #800",
        Comment.Special:           "nobold noitalic bg:#800",

        Keyword:                   "nobold noitalic #008",
        Keyword.Pseudo:            "nobold noitalic #008",
        Keyword.Type:              "nobold noitalic #902000",

        Operator:                  "nobold noitalic #660",
        Operator.Word:             "nobold noitalic #660",

        Name.Builtin:              "nobold noitalic #008",
        Name.Function:             "nobold noitalic #000",
        Name.Class:                "nobold noitalic #606",
        Name.Namespace:            "nobold noitalic #000",
        Name.Exception:            "nobold noitalic #000",
        Name.Variable:             "nobold noitalic #000",
        Name.Constant:             "nobold noitalic #000",
        Name.Label:                "nobold noitalic #000",
        Name.Entity:               "nobold noitalic #000",
        Name.Attribute:            "nobold noitalic #000",
        Name.Tag:                  "nobold noitalic #000",
        Name.Decorator:            "nobold noitalic #000",

        String:                    "nobold noitalic #080",
        String.Doc:                "nobold noitalic #080",
        String.Interpol:           "nobold noitalic #080",
        String.Escape:             "nobold noitalic #080",
        String.Regex:              "nobold noitalic #080",
        String.Symbol:             "nobold noitalic #660",
        String.Other:              "nobold noitalic #080",
        Number:                    "nobold noitalic #080",

        Generic.Heading:           "nobold noitalic #000",
        Generic.Subheading:        "nobold noitalic #000",
        Generic.Deleted:           "nobold noitalic #000",
        Generic.Inserted:          "nobold noitalic #000",
        Generic.Error:             "nobold noitalic #000",
        Generic.Emph:              "nobold noitalic #000",
        Generic.Strong:            "nobold noitalic #000",
        Generic.Prompt:            "nobold noitalic #000",
        Generic.Output:            "nobold noitalic #000",
        Generic.Traceback:         "nobold noitalic #000",

        Error:                     "nobold noitalic border:#FF0000"
    }
