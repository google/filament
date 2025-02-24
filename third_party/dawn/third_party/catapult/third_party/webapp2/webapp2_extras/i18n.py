# -*- coding: utf-8 -*-
"""
    webapp2_extras.i18n
    ===================

    Internationalization support for webapp2.

    Several ideas borrowed from tipfy.i18n and Flask-Babel.

    :copyright: 2011 by tipfy.org.
    :license: Apache Sotware License, see LICENSE for details.
"""
import datetime
import gettext as gettext_stdlib

import babel
from babel import dates
from babel import numbers
from babel import support

try:
    # Monkeypatches pytz for gae.
    import pytz.gae
except ImportError: # pragma: no cover
    pass

import pytz

import webapp2

#: Default configuration values for this module. Keys are:
#:
#: translations_path
#:     Path to the translations directory. Default is `locale`.
#:
#: domains
#:     List of gettext domains to be used. Default is ``['messages']``.
#:
#: default_locale
#:     A locale code to be used as fallback. Default is ``'en_US'``.
#:
#: default_timezone
#:     The application default timezone according to the Olson
#:     database. Default is ``'UTC'``.
#:
#: locale_selector
#:     A function that receives (store, request) and returns a locale
#:     to be used for a request. If not defined, uses `default_locale`.
#:     Can also be a string in dotted notation to be imported.
#:
#: timezone_selector
#:     A function that receives (store, request) and returns a timezone
#:     to be used for a request. If not defined, uses `default_timezone`.
#:     Can also be a string in dotted notation to be imported.
#:
#: date_formats
#:     Default date formats for datetime, date and time.
default_config = {
    'translations_path':   'locale',
    'domains':             ['messages'],
    'default_locale':      'en_US',
    'default_timezone':    'UTC',
    'locale_selector':     None,
    'timezone_selector':   None,
    'date_formats': {
        'time':            'medium',
        'date':            'medium',
        'datetime':        'medium',
        'time.short':      None,
        'time.medium':     None,
        'time.full':       None,
        'time.long':       None,
        'time.iso':        "HH':'mm':'ss",
        'date.short':      None,
        'date.medium':     None,
        'date.full':       None,
        'date.long':       None,
        'date.iso':        "yyyy'-'MM'-'dd",
        'datetime.short':  None,
        'datetime.medium': None,
        'datetime.full':   None,
        'datetime.long':   None,
        'datetime.iso':    "yyyy'-'MM'-'dd'T'HH':'mm':'ssZ",
    },
}

NullTranslations = gettext_stdlib.NullTranslations


class I18nStore(object):
    """Internalization store.

    Caches loaded translations and configuration to be used between requests.
    """

    #: Configuration key.
    config_key = __name__
    #: A dictionary with all loaded translations.
    translations = None
    #: Path to where traslations are stored.
    translations_path = None
    #: Translation domains to merge.
    domains = None
    #: Default locale code.
    default_locale = None
    #: Default timezone code.
    default_timezone = None
    #: Dictionary of default date formats.
    date_formats = None
    #: A callable that returns the locale for a request.
    locale_selector = None
    #: A callable that returns the timezone for a request.
    timezone_selector = None

    def __init__(self, app, config=None):
        """Initializes the i18n store.

        :param app:
            A :class:`webapp2.WSGIApplication` instance.
        :param config:
            A dictionary of configuration values to be overridden. See
            the available keys in :data:`default_config`.
        """
        config = app.config.load_config(self.config_key,
            default_values=default_config, user_values=config,
            required_keys=None)
        self.translations = {}
        self.translations_path = config['translations_path']
        self.domains = config['domains']
        self.default_locale = config['default_locale']
        self.default_timezone = config['default_timezone']
        self.date_formats = config['date_formats']
        self.set_locale_selector(config['locale_selector'])
        self.set_timezone_selector(config['timezone_selector'])

    def set_locale_selector(self, func):
        """Sets the function that defines the locale for a request.

        :param func:
            A callable that receives (store, request) and returns the locale
            for a request.
        """
        if func is None:
            self.locale_selector = self.default_locale_selector
        else:
            if isinstance(func, basestring):
                func = webapp2.import_string(func)

            # Functions are descriptors, so bind it to this instance with
            # __get__.
            self.locale_selector = func.__get__(self, self.__class__)

    def set_timezone_selector(self, func):
        """Sets the function that defines the timezone for a request.

        :param func:
            A callable that receives (store, request) and returns the timezone
            for a request.
        """
        if func is None:
            self.timezone_selector = self.default_timezone_selector
        else:
            if isinstance(func, basestring):
                func = webapp2.import_string(func)

            self.timezone_selector = func.__get__(self, self.__class__)

    def default_locale_selector(self, request):
        return self.default_locale

    def default_timezone_selector(self, request):
        return self.default_timezone

    def get_translations(self, locale):
        """Returns a translation catalog for a locale.

        :param locale:
            A locale code.
        :returns:
            A ``babel.support.Translations`` instance, or
            ``gettext.NullTranslations`` if none was found.
        """
        trans = self.translations.get(locale)
        if not trans:
            locales = (locale, self.default_locale)
            trans = self.load_translations(self.translations_path, locales,
                                           self.domains)
            if not webapp2.get_app().debug:
                self.translations[locale] = trans

        return trans

    def load_translations(self, dirname, locales, domains):
        """Loads a translation catalog.

        :param dirname:
            Path to where translations are stored.
        :param locales:
            A list of locale codes.
        :param domains:
            A list of domains to be merged.
        :returns:
            A ``babel.support.Translations`` instance, or
            ``gettext.NullTranslations`` if none was found.
        """
        trans = None
        trans_null = None
        for domain in domains:
            _trans = support.Translations.load(dirname, locales, domain)
            if isinstance(_trans, NullTranslations):
                trans_null = _trans
                continue
            elif trans is None:
                trans = _trans
            else:
                trans.merge(_trans)

        return trans or trans_null or NullTranslations()


class I18n(object):
    """Internalization provider for a single request."""

    #: A reference to :class:`I18nStore`.
    store = None
    #: The current locale code.
    locale = None
    #: The current translations.
    translations = None
    #: The current timezone code.
    timezone = None
    #: The current tzinfo object.
    tzinfo = None

    def __init__(self, request):
        """Initializes the i18n provider for a request.

        :param request:
            A :class:`webapp2.Request` instance.
        """
        self.store = store = get_store(app=request.app)
        self.set_locale(store.locale_selector(request))
        self.set_timezone(store.timezone_selector(request))

    def set_locale(self, locale):
        """Sets the locale code for this request.

        :param locale:
            A locale code.
        """
        self.locale = locale
        self.translations = self.store.get_translations(locale)

    def set_timezone(self, timezone):
        """Sets the timezone code for this request.

        :param timezone:
            A timezone code.
        """
        self.timezone = timezone
        self.tzinfo = pytz.timezone(timezone)

    def gettext(self, string, **variables):
        """Translates a given string according to the current locale.

        :param string:
            The string to be translated.
        :param variables:
            Variables to format the returned string.
        :returns:
            The translated string.
        """
        if variables:
            return self.translations.ugettext(string) % variables

        return self.translations.ugettext(string)

    def ngettext(self, singular, plural, n, **variables):
        """Translates a possible pluralized string according to the current
        locale.

        :param singular:
            The singular for of the string to be translated.
        :param plural:
            The plural for of the string to be translated.
        :param n:
            An integer indicating if this is a singular or plural. If greater
            than 1, it is a plural.
        :param variables:
            Variables to format the returned string.
        :returns:
            The translated string.
        """
        if variables:
            return self.translations.ungettext(singular, plural, n) % variables

        return self.translations.ungettext(singular, plural, n)

    def to_local_timezone(self, datetime):
        """Returns a datetime object converted to the local timezone.

        :param datetime:
            A ``datetime`` object.
        :returns:
            A ``datetime`` object normalized to a timezone.
        """
        if datetime.tzinfo is None:
            datetime = datetime.replace(tzinfo=pytz.UTC)

        return self.tzinfo.normalize(datetime.astimezone(self.tzinfo))

    def to_utc(self, datetime):
        """Returns a datetime object converted to UTC and without tzinfo.

        :param datetime:
            A ``datetime`` object.
        :returns:
            A naive ``datetime`` object (no timezone), converted to UTC.
        """
        if datetime.tzinfo is None:
            datetime = self.tzinfo.localize(datetime)

        return datetime.astimezone(pytz.UTC).replace(tzinfo=None)

    def _get_format(self, key, format):
        """A helper for the datetime formatting functions. Returns a format
        name or pattern to be used by Babel date format functions.

        :param key:
            A format key to be get from config. Valid values are "date",
            "datetime" or "time".
        :param format:
            The format to be returned. Valid values are "short", "medium",
            "long", "full" or a custom date/time pattern.
        :returns:
            A format name or pattern to be used by Babel date format functions.
        """
        if format is None:
            format = self.store.date_formats.get(key)

        if format in ('short', 'medium', 'full', 'long', 'iso'):
            rv = self.store.date_formats.get('%s.%s' % (key, format))
            if rv is not None:
                format = rv

        return format

    def format_date(self, date=None, format=None, rebase=True):
        """Returns a date formatted according to the given pattern and
        following the current locale.

        :param date:
            A ``date`` or ``datetime`` object. If None, the current date in
            UTC is used.
        :param format:
            The format to be returned. Valid values are "short", "medium",
            "long", "full" or a custom date/time pattern. Example outputs:

            - short:  11/10/09
            - medium: Nov 10, 2009
            - long:   November 10, 2009
            - full:   Tuesday, November 10, 2009

        :param rebase:
            If True, converts the date to the current :attr:`timezone`.
        :returns:
            A formatted date in unicode.
        """
        format = self._get_format('date', format)

        if rebase and isinstance(date, datetime.datetime):
            date = self.to_local_timezone(date)

        return dates.format_date(date, format, locale=self.locale)

    def format_datetime(self, datetime=None, format=None, rebase=True):
        """Returns a date and time formatted according to the given pattern
        and following the current locale and timezone.

        :param datetime:
            A ``datetime`` object. If None, the current date and time in UTC
            is used.
        :param format:
            The format to be returned. Valid values are "short", "medium",
            "long", "full" or a custom date/time pattern. Example outputs:

            - short:  11/10/09 4:36 PM
            - medium: Nov 10, 2009 4:36:05 PM
            - long:   November 10, 2009 4:36:05 PM +0000
            - full:   Tuesday, November 10, 2009 4:36:05 PM World (GMT) Time

        :param rebase:
            If True, converts the datetime to the current :attr:`timezone`.
        :returns:
            A formatted date and time in unicode.
        """
        format = self._get_format('datetime', format)

        kwargs = {}
        if rebase:
            kwargs['tzinfo'] = self.tzinfo

        return dates.format_datetime(datetime, format, locale=self.locale,
                                     **kwargs)

    def format_time(self, time=None, format=None, rebase=True):
        """Returns a time formatted according to the given pattern and
        following the current locale and timezone.

        :param time:
            A ``time`` or ``datetime`` object. If None, the current
            time in UTC is used.
        :param format:
            The format to be returned. Valid values are "short", "medium",
            "long", "full" or a custom date/time pattern. Example outputs:

            - short:  4:36 PM
            - medium: 4:36:05 PM
            - long:   4:36:05 PM +0000
            - full:   4:36:05 PM World (GMT) Time

        :param rebase:
            If True, converts the time to the current :attr:`timezone`.
        :returns:
            A formatted time in unicode.
        """
        format = self._get_format('time', format)

        kwargs = {}
        if rebase:
            kwargs['tzinfo'] = self.tzinfo

        return dates.format_time(time, format, locale=self.locale, **kwargs)

    def format_timedelta(self, datetime_or_timedelta, granularity='second',
                         threshold=.85):
        """Formats the elapsed time from the given date to now or the given
        timedelta. This currently requires an unreleased development version
        of Babel.

        :param datetime_or_timedelta:
            A ``timedelta`` object representing the time difference to format,
            or a ``datetime`` object in UTC.
        :param granularity:
            Determines the smallest unit that should be displayed, the value
            can be one of "year", "month", "week", "day", "hour", "minute" or
            "second".
        :param threshold:
            Factor that determines at which point the presentation switches to
            the next higher unit.
        :returns:
            A string with the elapsed time.
        """
        if isinstance(datetime_or_timedelta, datetime.datetime):
            datetime_or_timedelta = datetime.datetime.utcnow() - \
                datetime_or_timedelta

        return dates.format_timedelta(datetime_or_timedelta, granularity,
                                      threshold=threshold,
                                      locale=self.locale)

    def format_number(self, number):
        """Returns the given number formatted for the current locale. Example::

            >>> format_number(1099, locale='en_US')
            u'1,099'

        :param number:
            The number to format.
        :returns:
            The formatted number.
        """
        return numbers.format_number(number, locale=self.locale)

    def format_decimal(self, number, format=None):
        """Returns the given decimal number formatted for the current locale.
        Example::

            >>> format_decimal(1.2345, locale='en_US')
            u'1.234'
            >>> format_decimal(1.2346, locale='en_US')
            u'1.235'
            >>> format_decimal(-1.2346, locale='en_US')
            u'-1.235'
            >>> format_decimal(1.2345, locale='sv_SE')
            u'1,234'
            >>> format_decimal(12345, locale='de')
            u'12.345'

        The appropriate thousands grouping and the decimal separator are used
        for each locale::

            >>> format_decimal(12345.5, locale='en_US')
            u'12,345.5'

        :param number:
            The number to format.
        :param format:
            Notation format.
        :returns:
            The formatted decimal number.
        """
        return numbers.format_decimal(number, format=format,
                                      locale=self.locale)

    def format_currency(self, number, currency, format=None):
        """Returns a formatted currency value. Example::

            >>> format_currency(1099.98, 'USD', locale='en_US')
            u'$1,099.98'
            >>> format_currency(1099.98, 'USD', locale='es_CO')
            u'US$\\xa01.099,98'
            >>> format_currency(1099.98, 'EUR', locale='de_DE')
            u'1.099,98\\xa0\\u20ac'

        The pattern can also be specified explicitly::

            >>> format_currency(1099.98, 'EUR', u'\\xa4\\xa4 #,##0.00',
            ...                 locale='en_US')
            u'EUR 1,099.98'

        :param number:
            The number to format.
        :param currency:
            The currency code.
        :param format:
            Notation format.
        :returns:
            The formatted currency value.
        """
        return numbers.format_currency(number, currency, format=format,
                                       locale=self.locale)

    def format_percent(self, number, format=None):
        """Returns formatted percent value for the current locale. Example::

            >>> format_percent(0.34, locale='en_US')
            u'34%'
            >>> format_percent(25.1234, locale='en_US')
            u'2,512%'
            >>> format_percent(25.1234, locale='sv_SE')
            u'2\\xa0512\\xa0%'

        The format pattern can also be specified explicitly::

            >>> format_percent(25.1234, u'#,##0\u2030', locale='en_US')
            u'25,123\u2030'

        :param number:
            The percent number to format
        :param format:
            Notation format.
        :returns:
            The formatted percent number.
        """
        return numbers.format_percent(number, format=format,
                                      locale=self.locale)

    def format_scientific(self, number, format=None):
        """Returns value formatted in scientific notation for the current
        locale. Example::

            >>> format_scientific(10000, locale='en_US')
            u'1E4'

        The format pattern can also be specified explicitly::

            >>> format_scientific(1234567, u'##0E00', locale='en_US')
            u'1.23E06'

        :param number:
            The number to format.
        :param format:
            Notation format.
        :returns:
            Value formatted in scientific notation.
        """
        return numbers.format_scientific(number, format=format,
                                         locale=self.locale)

    def parse_date(self, string):
        """Parses a date from a string.

        This function uses the date format for the locale as a hint to
        determine the order in which the date fields appear in the string.
        Example::

            >>> parse_date('4/1/04', locale='en_US')
            datetime.date(2004, 4, 1)
            >>> parse_date('01.04.2004', locale='de_DE')
            datetime.date(2004, 4, 1)

        :param string:
            The string containing the date.
        :returns:
            The parsed date object.
        """
        return dates.parse_date(string, locale=self.locale)

    def parse_datetime(self, string):
        """Parses a date and time from a string.

        This function uses the date and time formats for the locale as a hint
        to determine the order in which the time fields appear in the string.

        :param string:
            The string containing the date and time.
        :returns:
            The parsed datetime object.
        """
        return dates.parse_datetime(string, locale=self.locale)

    def parse_time(self, string):
        """Parses a time from a string.

        This function uses the time format for the locale as a hint to
        determine the order in which the time fields appear in the string.
        Example::

            >>> parse_time('15:30:00', locale='en_US')
            datetime.time(15, 30)

        :param string:
            The string containing the time.
        :returns:
            The parsed time object.
        """
        return dates.parse_time(string, locale=self.locale)

    def parse_number(self, string):
        """Parses localized number string into a long integer. Example::

            >>> parse_number('1,099', locale='en_US')
            1099L
            >>> parse_number('1.099', locale='de_DE')
            1099L

        When the given string cannot be parsed, an exception is raised::

            >>> parse_number('1.099,98', locale='de')
            Traceback (most recent call last):
               ...
            NumberFormatError: '1.099,98' is not a valid number

        :param string:
            The string to parse.
        :returns:
            The parsed number.
        :raises:
            ``NumberFormatError`` if the string can not be converted to a
            number.
        """
        return numbers.parse_number(string, locale=self.locale)

    def parse_decimal(self, string):
        """Parses localized decimal string into a float. Example::

            >>> parse_decimal('1,099.98', locale='en_US')
            1099.98
            >>> parse_decimal('1.099,98', locale='de')
            1099.98

        When the given string cannot be parsed, an exception is raised::

            >>> parse_decimal('2,109,998', locale='de')
            Traceback (most recent call last):
               ...
            NumberFormatError: '2,109,998' is not a valid decimal number

        :param string:
            The string to parse.
        :returns:
            The parsed decimal number.
        :raises:
            ``NumberFormatError`` if the string can not be converted to a
            decimal number.
        """
        return numbers.parse_decimal(string, locale=self.locale)

    def get_timezone_location(self, dt_or_tzinfo):
        """Returns a representation of the given timezone using "location
        format".

        The result depends on both the local display name of the country and
        the city assocaited with the time zone::

            >>> from pytz import timezone
            >>> tz = timezone('America/St_Johns')
            >>> get_timezone_location(tz, locale='de_DE')
            u"Kanada (St. John's)"
            >>> tz = timezone('America/Mexico_City')
            >>> get_timezone_location(tz, locale='de_DE')
            u'Mexiko (Mexiko-Stadt)'

        If the timezone is associated with a country that uses only a single
        timezone, just the localized country name is returned::

            >>> tz = timezone('Europe/Berlin')
            >>> get_timezone_name(tz, locale='de_DE')
            u'Deutschland'

        :param dt_or_tzinfo:
            The ``datetime`` or ``tzinfo`` object that determines
            the timezone; if None, the current date and time in UTC is assumed.
        :returns:
            The localized timezone name using location format.
        """
        return dates.get_timezone_name(dt_or_tzinfo, locale=self.locale)


def gettext(string, **variables):
    """See :meth:`I18n.gettext`."""
    return get_i18n().gettext(string, **variables)


def ngettext(singular, plural, n, **variables):
    """See :meth:`I18n.ngettext`."""
    return get_i18n().ngettext(singular, plural, n, **variables)


def to_local_timezone(datetime):
    """See :meth:`I18n.to_local_timezone`."""
    return get_i18n().to_local_timezone(datetime)


def to_utc(datetime):
    """See :meth:`I18n.to_utc`."""
    return get_i18n().to_utc(datetime)


def format_date(date=None, format=None, rebase=True):
    """See :meth:`I18n.format_date`."""
    return get_i18n().format_date(date, format, rebase)


def format_datetime(datetime=None, format=None, rebase=True):
    """See :meth:`I18n.format_datetime`."""
    return get_i18n().format_datetime(datetime, format, rebase)


def format_time(time=None, format=None, rebase=True):
    """See :meth:`I18n.format_time`."""
    return get_i18n().format_time(time, format, rebase)


def format_timedelta(datetime_or_timedelta, granularity='second',
    threshold=.85):
    """See :meth:`I18n.format_timedelta`."""
    return get_i18n().format_timedelta(datetime_or_timedelta,
                                       granularity, threshold)


def format_number(number):
    """See :meth:`I18n.format_number`."""
    return get_i18n().format_number(number)


def format_decimal(number, format=None):
    """See :meth:`I18n.format_decimal`."""
    return get_i18n().format_decimal(number, format)


def format_currency(number, currency, format=None):
    """See :meth:`I18n.format_currency`."""
    return get_i18n().format_currency(number, currency, format)


def format_percent(number, format=None):
    """See :meth:`I18n.format_percent`."""
    return get_i18n().format_percent(number, format)


def format_scientific(number, format=None):
    """See :meth:`I18n.format_scientific`."""
    return get_i18n().format_scientific(number, format)


def parse_date(string):
    """See :meth:`I18n.parse_date`"""
    return get_i18n().parse_date(string)


def parse_datetime(string):
    """See :meth:`I18n.parse_datetime`."""
    return get_i18n().parse_datetime(string)


def parse_time(string):
    """See :meth:`I18n.parse_time`."""
    return get_i18n().parse_time(string)


def parse_number(string):
    """See :meth:`I18n.parse_number`."""
    return get_i18n().parse_number(string)


def parse_decimal(string):
    """See :meth:`I18n.parse_decimal`."""
    return get_i18n().parse_decimal(string)


def get_timezone_location(dt_or_tzinfo):
    """See :meth:`I18n.get_timezone_location`."""
    return get_i18n().get_timezone_location(dt_or_tzinfo)


def lazy_gettext(string, **variables):
    """A lazy version of :func:`gettext`.

    :param string:
        The string to be translated.
    :param variables:
        Variables to format the returned string.
    :returns:
        A ``babel.support.LazyProxy`` object that when accessed translates
        the string.
    """
    return support.LazyProxy(gettext, string, **variables)


# Aliases.
_ = gettext
_lazy = lazy_gettext


# Factories -------------------------------------------------------------------


#: Key used to store :class:`I18nStore` in the app registry.
_store_registry_key = 'webapp2_extras.i18n.I18nStore'
#: Key used to store :class:`I18n` in the request registry.
_i18n_registry_key = 'webapp2_extras.i18n.I18n'


def get_store(factory=I18nStore, key=_store_registry_key, app=None):
    """Returns an instance of :class:`I18nStore` from the app registry.

    It'll try to get it from the current app registry, and if it is not
    registered it'll be instantiated and registered. A second call to this
    function will return the same instance.

    :param factory:
        The callable used to build and register the instance if it is not yet
        registered. The default is the class :class:`I18nStore` itself.
    :param key:
        The key used to store the instance in the registry. A default is used
        if it is not set.
    :param app:
        A :class:`webapp2.WSGIApplication` instance used to store the instance.
        The active app is used if it is not set.
    """
    app = app or webapp2.get_app()
    store = app.registry.get(key)
    if not store:
        store = app.registry[key] = factory(app)

    return store


def set_store(store, key=_store_registry_key, app=None):
    """Sets an instance of :class:`I18nStore` in the app registry.

    :param store:
        An instance of :class:`I18nStore`.
    :param key:
        The key used to retrieve the instance from the registry. A default
        is used if it is not set.
    :param request:
        A :class:`webapp2.WSGIApplication` instance used to retrieve the
        instance. The active app is used if it is not set.
    """
    app = app or webapp2.get_app()
    app.registry[key] = store


def get_i18n(factory=I18n, key=_i18n_registry_key, request=None):
    """Returns an instance of :class:`I18n` from the request registry.

    It'll try to get it from the current request registry, and if it is not
    registered it'll be instantiated and registered. A second call to this
    function will return the same instance.

    :param factory:
        The callable used to build and register the instance if it is not yet
        registered. The default is the class :class:`I18n` itself.
    :param key:
        The key used to store the instance in the registry. A default is used
        if it is not set.
    :param request:
        A :class:`webapp2.Request` instance used to store the instance. The
        active request is used if it is not set.
    """
    request = request or webapp2.get_request()
    i18n = request.registry.get(key)
    if not i18n:
        i18n = request.registry[key] = factory(request)

    return i18n


def set_i18n(i18n, key=_i18n_registry_key, request=None):
    """Sets an instance of :class:`I18n` in the request registry.

    :param store:
        An instance of :class:`I18n`.
    :param key:
        The key used to retrieve the instance from the registry. A default
        is used if it is not set.
    :param request:
        A :class:`webapp2.Request` instance used to retrieve the instance. The
        active request is used if it is not set.
    """
    request = request or webapp2.get_request()
    request.registry[key] = i18n
