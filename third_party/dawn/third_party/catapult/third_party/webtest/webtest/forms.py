# -*- coding: utf-8 -*-
"""Helpers to fill and submit forms."""

import re
import sys

from bs4 import BeautifulSoup
from webtest.compat import OrderedDict
from webtest import utils


class NoValue(object):
    pass


class Upload(object):
    """
    A file to upload::

        >>> Upload('filename.txt', 'data', 'application/octet-stream')
        <Upload "filename.txt">
        >>> Upload('filename.txt', 'data')
        <Upload "filename.txt">
        >>> Upload("README.txt")
        <Upload "README.txt">

    :param filename: Name of the file to upload.
    :param content: Contents of the file.
    :param content_type: MIME type of the file.

    """

    def __init__(self, filename, content=None, content_type=None):
        self.filename = filename
        self.content = content
        self.content_type = content_type

    def __iter__(self):
        yield self.filename
        if self.content:
            yield self.content
            yield self.content_type
        # TODO: do we handle the case when we need to get
        # contents ourselves?

    def __repr__(self):
        return '<Upload "%s">' % self.filename


class Field(object):
    """Base class for all Field objects.

    .. attribute:: classes

        Dictionary of field types (select, radio, etc)

    .. attribute:: value

        Set/get value of the field.

    """

    classes = {}

    def __init__(self, form, tag, name, pos,
                 value=None, id=None, **attrs):
        self.form = form
        self.tag = tag
        self.name = name
        self.pos = pos
        self._value = value
        self.id = id
        self.attrs = attrs

    def value__get(self):
        if self._value is None:
            return ''
        else:
            return self._value

    def value__set(self, value):
        self._value = value

    value = property(value__get, value__set)

    def force_value(self, value):
        """Like setting a value, except forces it (even for, say, hidden
        fields).
        """
        self._value = value

    def __repr__(self):
        value = '<%s name="%s"' % (self.__class__.__name__, self.name)
        if self.id:
            value += ' id="%s"' % self.id
        return value + '>'


class Select(Field):
    """Field representing ``<select />`` form element."""

    def __init__(self, *args, **attrs):
        super(Select, self).__init__(*args, **attrs)
        self.options = []
        # Undetermined yet:
        self.selectedIndex = None
        # we have no forced value
        self._forced_value = NoValue

    def force_value(self, value):
        """Like setting a value, except forces it (even for, say, hidden
        fields).
        """
        self._forced_value = value

    def select(self, value=None, text=None):
        if value is not None and text is not None:
            raise ValueError("Specify only one of value and text.")

        if text is not None:
            value = self._get_value_for_text(text)

        self.value = value

    def _get_value_for_text(self, text):
        for i, (option_value, checked, option_text) in enumerate(self.options):
            if option_text == utils.stringify(text):
                return option_value

        raise ValueError("Option with text %r not found (from %s)"
                         % (text, ', '.join(
                             [repr(t) for o, c, t in self.options])))

    def value__set(self, value):
        if self._forced_value is not NoValue:
            self._forced_value = NoValue
        for i, (option, checked, text) in enumerate(self.options):
            if option == utils.stringify(value):
                self.selectedIndex = i
                break
        else:
            raise ValueError(
                "Option %r not found (from %s)"
                % (value, ', '.join([repr(o) for o, c, t in self.options])))

    def value__get(self):
        if self._forced_value is not NoValue:
            return self._forced_value
        elif self.selectedIndex is not None:
            return self.options[self.selectedIndex][0]
        else:
            for option, checked, text in self.options:
                if checked:
                    return option
            else:
                if self.options:
                    return self.options[0][0]

    value = property(value__get, value__set)


class MultipleSelect(Field):
    """Field representing ``<select multiple="multiple">``"""

    def __init__(self, *args, **attrs):
        super(MultipleSelect, self).__init__(*args, **attrs)
        self.options = []
        # Undetermined yet:
        self.selectedIndices = []
        self._forced_values = []

    def force_value(self, values):
        """Like setting a value, except forces it (even for, say, hidden
        fields).
        """
        self._forced_values = values
        self.selectedIndices = []

    def select_multiple(self, value=None, texts=None):
        if value is not None and texts is not None:
            raise ValueError("Specify only one of value and texts.")

        if texts is not None:
            value = self._get_value_for_texts(texts)

        self.value = value

    def _get_value_for_texts(self, texts):
        str_texts = [utils.stringify(text) for text in texts]
        value = []
        for i, (option, checked, text) in enumerate(self.options):
            if text in str_texts:
                value.append(option)
                str_texts.remove(text)

        if str_texts:
            raise ValueError(
                "Option(s) %r not found (from %s)"
                % (', '.join(str_texts),
                   ', '.join([repr(t) for o, c, t in self.options])))

        return value

    def value__set(self, values):
        str_values = [utils.stringify(value) for value in values]
        self.selectedIndices = []
        for i, (option, checked, text) in enumerate(self.options):
            if option in str_values:
                self.selectedIndices.append(i)
                str_values.remove(option)
        if str_values:
            raise ValueError(
                "Option(s) %r not found (from %s)"
                % (', '.join(str_values),
                   ', '.join([repr(o) for o, c, t in self.options])))

    def value__get(self):
        selected_values = []
        if self.selectedIndices:
            selected_values = [self.options[i][0]
                               for i in self.selectedIndices]
        elif not self._forced_values:
            selected_values = []
            for option, checked, text in self.options:
                if checked:
                    selected_values.append(option)
        if self._forced_values:
            selected_values += self._forced_values

        if self.options and (not selected_values):
            selected_values = None
        return selected_values
    value = property(value__get, value__set)


class Radio(Select):
    """Field representing ``<input type="radio">``"""

    def value__get(self):
        if self._forced_value is not NoValue:
            return self._forced_value
        elif self.selectedIndex is not None:
            return self.options[self.selectedIndex][0]
        else:
            for option, checked, text in self.options:
                if checked:
                    return option
            else:
                return None

    value = property(value__get, Select.value__set)


class Checkbox(Field):
    """Field representing ``<input type="checkbox">``

    .. attribute:: checked

        Returns True if checkbox is checked.

    """

    def __init__(self, *args, **attrs):
        super(Checkbox, self).__init__(*args, **attrs)
        self._checked = 'checked' in attrs

    def value__set(self, value):
        self._checked = not not value

    def value__get(self):
        if self._checked:
            if self._value is None:
                return 'on'
            else:
                return self._value
        else:
            return None

    value = property(value__get, value__set)

    def checked__get(self):
        return bool(self._checked)

    def checked__set(self, value):
        self._checked = not not value

    checked = property(checked__get, checked__set)


class Text(Field):
    """Field representing ``<input type="text">``"""


class File(Field):
    """Field representing ``<input type="file">``"""

    # TODO: This doesn't actually handle file uploads and enctype
    def value__get(self):
        if self._value is None:
            return ''
        else:
            return self._value

    value = property(value__get, Field.value__set)


class Textarea(Text):
    """Field representing ``<textarea>``"""


class Hidden(Text):
    """Field representing ``<input type="hidden">``"""


class Submit(Field):
    """Field representing ``<input type="submit">`` and ``<button>``"""

    def value__get(self):
        return None

    def value__set(self, value):
        raise AttributeError(
            "You cannot set the value of the <%s> field %r"
            % (self.tag, self.name))

    value = property(value__get, value__set)

    def value_if_submitted(self):
        # TODO: does this ever get set?
        return self._value


Field.classes['submit'] = Submit

Field.classes['button'] = Submit

Field.classes['image'] = Submit

Field.classes['multiple_select'] = MultipleSelect

Field.classes['select'] = Select

Field.classes['hidden'] = Hidden

Field.classes['file'] = File

Field.classes['text'] = Text

Field.classes['password'] = Text

Field.classes['checkbox'] = Checkbox

Field.classes['textarea'] = Textarea

Field.classes['radio'] = Radio


class Form(object):
    """This object represents a form that has been found in a page.

    :param response: `webob.response.TestResponse` instance
    :param text: Unparsed html of the form

    .. attribute:: text

        the full HTML of the form.

    .. attribute:: action

        the relative URI of the action.

    .. attribute:: method

        the HTTP method (e.g., ``'GET'``).

    .. attribute:: id

        the id, or None if not given.

    .. attribute:: enctype

        encoding of the form submission

    .. attribute:: fields

        a dictionary of fields, each value is a list of fields by
        that name.  ``<input type=\"radio\">`` and ``<select>`` are
        both represented as single fields with multiple options.

    .. attribute:: field_order

        Ordered list of field names as found in the html.

    """

    # TODO: use BeautifulSoup4 for this

    _tag_re = re.compile(r'<(/?)([a-z0-9_\-]*)([^>]*?)>', re.I)
    _label_re = re.compile(
        '''<label\s+(?:[^>]*)for=(?:"|')([a-z0-9_\-]+)(?:"|')(?:[^>]*)>''',
        re.I)

    FieldClass = Field

    def __init__(self, response, text, parser_features='html.parser'):
        self.response = response
        self.text = text
        self.html = BeautifulSoup(self.text, parser_features)

        attrs = self.html('form')[0].attrs
        self.action = attrs.get('action', '')
        self.method = attrs.get('method', 'GET')
        self.id = attrs.get('id')
        self.enctype = attrs.get('enctype',
                                 'application/x-www-form-urlencoded')

        self._parse_fields()

    def _parse_fields(self):
        fields = OrderedDict()
        field_order = []
        tags = ('input', 'select', 'textarea', 'button')
        for pos, node in enumerate(self.html.findAll(tags)):
            attrs = dict(node.attrs)
            tag = node.name
            name = None
            if 'name' in attrs:
                name = attrs.pop('name')

            if tag == 'textarea':
                if node.text.startswith('\r\n'):  # pragma: no cover
                    text = node.text[2:]
                elif node.text.startswith('\n'):
                    text = node.text[1:]
                else:
                    text = node.text
                attrs['value'] = text

            tag_type = attrs.get('type', 'text').lower()
            if tag == 'select':
                tag_type = 'select'
            if tag_type == "select" and "multiple" in attrs:
                tag_type = "multiple_select"
            if tag == 'button':
                tag_type = 'submit'

            FieldClass = self.FieldClass.classes.get(tag_type,
                                                     self.FieldClass)

            # https://github.com/Pylons/webtest/issues/73
            if sys.version_info[:2] <= (2, 6):
                attrs = dict((k.encode('utf-8') if isinstance(k, unicode)
                              else k, v) for k, v in attrs.items())

            # https://github.com/Pylons/webtest/issues/131
            reserved_attributes = ('form', 'tag', 'pos')
            for attr in reserved_attributes:
                if attr in attrs:
                    del attrs[attr]

            if tag == 'input':
                if tag_type == 'radio':
                    field = fields.get(name)
                    if not field:
                        field = FieldClass(self, tag, name, pos, **attrs)
                        fields.setdefault(name, []).append(field)
                        field_order.append((name, field))
                    else:
                        field = field[0]
                        assert isinstance(field,
                                          self.FieldClass.classes['radio'])
                    field.options.append((attrs.get('value'),
                                          'checked' in attrs,
                                          None))
                    continue
                elif tag_type == 'file':
                    if 'value' in attrs:
                        del attrs['value']

            field = FieldClass(self, tag, name, pos, **attrs)
            fields.setdefault(name, []).append(field)
            field_order.append((name, field))

            if tag == 'select':
                for option in node('option'):
                    field.options.append(
                        (option.attrs.get('value', option.text),
                         'selected' in option.attrs,
                         option.text))

        self.field_order = field_order
        self.fields = fields

    def __setitem__(self, name, value):
        """Set the value of the named field. If there is 0 or multiple fields
        by that name, it is an error.

        Multiple checkboxes of the same name are special-cased; a list may be
        assigned to them to check the checkboxes whose value is present in the
        list (and uncheck all others).

        Setting the value of a ``<select>`` selects the given option (and
        confirms it is an option). Setting radio fields does the same.
        Checkboxes get boolean values. You cannot set hidden fields or buttons.

        Use ``.set()`` if there is any ambiguity and you must provide an index.
        """
        fields = self.fields.get(name)
        assert fields is not None, (
            "No field by the name %r found (fields: %s)"
            % (name, ', '.join(map(repr, self.fields.keys()))))
        all_checkboxes = all(isinstance(f, Checkbox) for f in fields)
        if all_checkboxes and isinstance(value, list):
            values = set(utils.stringify(v) for v in value)
            for f in fields:
                f.checked = f._value in values
        else:
            assert len(fields) == 1, (
                "Multiple fields match %r: %s"
                % (name, ', '.join(map(repr, fields))))
            fields[0].value = value

    def __getitem__(self, name):
        """Get the named field object (ambiguity is an error)."""
        fields = self.fields.get(name)
        assert fields is not None, (
            "No field by the name %r found" % name)
        assert len(fields) == 1, (
            "Multiple fields match %r: %s"
            % (name, ', '.join(map(repr, fields))))
        return fields[0]

    def lint(self):
        """
        Check that the html is valid:

        - each field must have an id
        - each field must have a label

        """
        labels = self._label_re.findall(self.text)
        for name, fields in self.fields.items():
            for field in fields:
                if not isinstance(field, (Submit, Hidden)):
                    if not field.id:
                        raise AttributeError("%r as no id attribute" % field)
                    elif field.id not in labels:
                        raise AttributeError(
                            "%r as no associated label" % field)

    def set(self, name, value, index=None):
        """Set the given name, using ``index`` to disambiguate."""
        if index is None:
            self[name] = value
        else:
            fields = self.fields.get(name)
            assert fields is not None, (
                "No fields found matching %r" % name)
            field = fields[index]
            field.value = value

    def get(self, name, index=None, default=utils.NoDefault):
        """
        Get the named/indexed field object, or ``default`` if no field is
        found. Throws an AssertionError if no field is found and no ``default``
        was given.
        """
        fields = self.fields.get(name)
        if fields is None:
            if default is utils.NoDefault:
                raise AssertionError(
                    "No fields found matching %r (and no default given)"
                    % name)
            return default
        if index is None:
            return self[name]
        return fields[index]

    def select(self, name, value=None, text=None, index=None):
        """Like ``.set()``, except also confirms the target is a ``<select>``
        and allows selecting options by text.
        """
        field = self.get(name, index=index)
        assert isinstance(field, Select)

        field.select(value, text)

    def select_multiple(self, name, value=None, texts=None, index=None):
        """Like ``.set()``, except also confirms the target is a
        ``<select multiple>`` and allows selecting options by text.
        """
        field = self.get(name, index=index)
        assert isinstance(field, MultipleSelect)

        field.select_multiple(value, texts)

    def submit(self, name=None, index=None, value=None, **args):
        """Submits the form.  If ``name`` is given, then also select that
        button (using ``index`` or ``value`` to disambiguate)``.

        Any extra keyword arguments are passed to the
        :meth:`webtest.TestResponse.get` or
        :meth:`webtest.TestResponse.post` method.

        Returns a :class:`webtest.TestResponse` object.

        """
        fields = self.submit_fields(name, index=index, submit_value=value)
        if self.method.upper() != "GET":
            args.setdefault("content_type",  self.enctype)
        return self.response.goto(self.action, method=self.method,
                                  params=fields, **args)

    def upload_fields(self):
        """Return a list of file field tuples of the form::

            (field name, file name)

        or::

            (field name, file name, file contents).

        """
        uploads = []
        for name, fields in self.fields.items():
            for field in fields:
                if isinstance(field, File) and field.value:
                    uploads.append([name] + list(field.value))
        return uploads

    def submit_fields(self, name=None, index=None, submit_value=None):
        """Return a list of ``[(name, value), ...]`` for the current state of
        the form.

        :param name: Same as for :meth:`submit`
        :param index: Same as for :meth:`submit`

        """
        submit = []
        # Use another name here so we can keep function param the same for BWC.
        submit_name = name
        if index is not None and submit_value is not None:
            raise ValueError("Can't specify both submit_value and index.")

        # If no particular button was selected, use the first one
        if index is None and submit_value is None:
            index = 0

        # This counts all fields with the submit name not just submit fields.
        current_index = 0
        for name, field in self.field_order:
            if name is None:  # pragma: no cover
                continue
            if submit_name is not None and name == submit_name:
                if index is not None and current_index == index:
                    submit.append((name, field.value_if_submitted()))
                if submit_value is not None and \
                   field.value_if_submitted() == submit_value:
                    submit.append((name, field.value_if_submitted()))
                current_index += 1
            else:
                value = field.value
                if value is None:
                    continue
                if isinstance(field, File):
                    submit.append((name, field))
                    continue
                if isinstance(value, list):
                    for item in value:
                        submit.append((name, item))
                else:
                    submit.append((name, value))
        return submit

    def __repr__(self):
        value = '<Form'
        if self.id:
            value += ' id=%r' % str(self.id)
        return value + ' />'
