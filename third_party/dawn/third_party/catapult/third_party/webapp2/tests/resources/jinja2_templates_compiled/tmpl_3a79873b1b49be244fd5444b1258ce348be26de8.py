from __future__ import division
from jinja2.runtime import LoopContext, TemplateReference, Macro, Markup, TemplateRuntimeError, missing, concat, escape, markup_join, unicode_join, to_string, TemplateNotFound
name = 'template1.html'

def root(context):
    l_message = context.resolve('message')
    if 0: yield None
    yield to_string(l_message)

blocks = {}
debug_info = '1=8'