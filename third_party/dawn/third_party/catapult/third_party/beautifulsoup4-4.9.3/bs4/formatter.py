from bs4.dammit import EntitySubstitution

class Formatter(EntitySubstitution):
    """Describes a strategy to use when outputting a parse tree to a string.

    Some parts of this strategy come from the distinction between
    HTML4, HTML5, and XML. Others are configurable by the user.

    Formatters are passed in as the `formatter` argument to methods
    like `PageElement.encode`. Most people won't need to think about
    formatters, and most people who need to think about them can pass
    in one of these predefined strings as `formatter` rather than
    making a new Formatter object:

    For HTML documents:
     * 'html' - HTML entity substitution for generic HTML documents. (default)
     * 'html5' - HTML entity substitution for HTML5 documents.
     * 'minimal' - Only make the substitutions necessary to guarantee
                   valid HTML.
     * None - Do not perform any substitution. This will be faster
              but may result in invalid markup.

    For XML documents:
     * 'html' - Entity substitution for XHTML documents.
     * 'minimal' - Only make the substitutions necessary to guarantee
                   valid XML. (default)
     * None - Do not perform any substitution. This will be faster
              but may result in invalid markup.
    """
    # Registries of XML and HTML formatters.
    XML_FORMATTERS = {}
    HTML_FORMATTERS = {}

    HTML = 'html'
    XML = 'xml'

    HTML_DEFAULTS = dict(
        cdata_containing_tags=set(["script", "style"]),
    )

    def _default(self, language, value, kwarg):
        if value is not None:
            return value
        if language == self.XML:
            return set()
        return self.HTML_DEFAULTS[kwarg]

    def __init__(
            self, language=None, entity_substitution=None,
            void_element_close_prefix='/', cdata_containing_tags=None,
    ):
        """Constructor.

        :param language: This should be Formatter.XML if you are formatting
           XML markup and Formatter.HTML if you are formatting HTML markup.

        :param entity_substitution: A function to call to replace special
           characters with XML/HTML entities. For examples, see 
           bs4.dammit.EntitySubstitution.substitute_html and substitute_xml.
        :param void_element_close_prefix: By default, void elements
           are represented as <tag/> (XML rules) rather than <tag>
           (HTML rules). To get <tag>, pass in the empty string.
        :param cdata_containing_tags: The list of tags that are defined
           as containing CDATA in this dialect. For example, in HTML,
           <script> and <style> tags are defined as containing CDATA,
           and their contents should not be formatted.
        """
        self.language = language
        self.entity_substitution = entity_substitution
        self.void_element_close_prefix = void_element_close_prefix
        self.cdata_containing_tags = self._default(
            language, cdata_containing_tags, 'cdata_containing_tags'
        )
            
    def substitute(self, ns):
        """Process a string that needs to undergo entity substitution.
        This may be a string encountered in an attribute value or as
        text.

        :param ns: A string.
        :return: A string with certain characters replaced by named
           or numeric entities.
        """
        if not self.entity_substitution:
            return ns
        from element import NavigableString
        if (isinstance(ns, NavigableString)
            and ns.parent is not None
            and ns.parent.name in self.cdata_containing_tags):
            # Do nothing.
            return ns
        # Substitute.
        return self.entity_substitution(ns)

    def attribute_value(self, value):
        """Process the value of an attribute.

        :param ns: A string.
        :return: A string with certain characters replaced by named
           or numeric entities.
        """
        return self.substitute(value)
    
    def attributes(self, tag):
        """Reorder a tag's attributes however you want.
        
        By default, attributes are sorted alphabetically. This makes
        behavior consistent between Python 2 and Python 3, and preserves
        backwards compatibility with older versions of Beautiful Soup.
        """
        if tag.attrs is None:
            return []
        return sorted(tag.attrs.items())

   
class HTMLFormatter(Formatter):
    """A generic Formatter for HTML."""
    REGISTRY = {}
    def __init__(self, *args, **kwargs):
        return super(HTMLFormatter, self).__init__(self.HTML, *args, **kwargs)

    
class XMLFormatter(Formatter):
    """A generic Formatter for XML."""
    REGISTRY = {}
    def __init__(self, *args, **kwargs):
        return super(XMLFormatter, self).__init__(self.XML, *args, **kwargs)


# Set up aliases for the default formatters.
HTMLFormatter.REGISTRY['html'] = HTMLFormatter(
    entity_substitution=EntitySubstitution.substitute_html
)
HTMLFormatter.REGISTRY["html5"] = HTMLFormatter(
    entity_substitution=EntitySubstitution.substitute_html,
    void_element_close_prefix = None
)
HTMLFormatter.REGISTRY["minimal"] = HTMLFormatter(
    entity_substitution=EntitySubstitution.substitute_xml
)
HTMLFormatter.REGISTRY[None] = HTMLFormatter(
    entity_substitution=None
)
XMLFormatter.REGISTRY["html"] =  XMLFormatter(
    entity_substitution=EntitySubstitution.substitute_html
)
XMLFormatter.REGISTRY["minimal"] = XMLFormatter(
    entity_substitution=EntitySubstitution.substitute_xml
)
XMLFormatter.REGISTRY[None] = Formatter(
    Formatter(Formatter.XML, entity_substitution=None)
)
