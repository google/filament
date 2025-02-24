#!/usr/bin/env python
"""Model Datastore Input Reader implementation for the map_job API."""
import copy

from google.appengine.ext import ndb

from google.appengine.ext import db
from mapreduce import datastore_range_iterators as db_iters
from mapreduce import errors
from mapreduce import namespace_range
from mapreduce import property_range
from mapreduce import util
from mapreduce.api.map_job import abstract_datastore_input_reader

# pylint: disable=invalid-name


class ModelDatastoreInputReader(abstract_datastore_input_reader
                                .AbstractDatastoreInputReader):
  """Implementation of an input reader for Datastore.

  Iterates over a Model and yields model instances.
  Supports both db.model and ndb.model.
  """

  _KEY_RANGE_ITER_CLS = db_iters.KeyRangeModelIterator

  @classmethod
  def _get_raw_entity_kind(cls, model_classpath):
    entity_type = util.for_name(model_classpath)
    if isinstance(entity_type, db.Model):
      return entity_type.kind()
    elif isinstance(entity_type, (ndb.Model, ndb.MetaModel)):
      # pylint: disable=protected-access
      return entity_type._get_kind()
    else:
      return util.get_short_name(model_classpath)

  @classmethod
  def split_input(cls, job_config):
    """Inherit docs."""
    params = job_config.input_reader_params
    shard_count = job_config.shard_count
    query_spec = cls._get_query_spec(params)

    if not property_range.should_shard_by_property_range(query_spec.filters):
      return super(ModelDatastoreInputReader, cls).split_input(job_config)

    p_range = property_range.PropertyRange(query_spec.filters,
                                           query_spec.model_class_path)
    p_ranges = p_range.split(shard_count)

    # User specified a namespace.
    if query_spec.ns:
      ns_range = namespace_range.NamespaceRange(
          namespace_start=query_spec.ns,
          namespace_end=query_spec.ns,
          _app=query_spec.app)
      ns_ranges = [copy.copy(ns_range) for _ in p_ranges]
    else:
      ns_keys = namespace_range.get_namespace_keys(
          query_spec.app, cls.MAX_NAMESPACES_FOR_KEY_SHARD+1)
      if not ns_keys:
        return
      # User doesn't specify ns but the number of ns is small.
      # We still split by property range.
      if len(ns_keys) <= cls.MAX_NAMESPACES_FOR_KEY_SHARD:
        ns_ranges = [namespace_range.NamespaceRange(_app=query_spec.app)
                     for _ in p_ranges]
      # Lots of namespaces. Split by ns.
      else:
        ns_ranges = namespace_range.NamespaceRange.split(n=shard_count,
                                                         contiguous=False,
                                                         can_query=lambda: True,
                                                         _app=query_spec.app)
        p_ranges = [copy.copy(p_range) for _ in ns_ranges]

    assert len(p_ranges) == len(ns_ranges)

    iters = [
        db_iters.RangeIteratorFactory.create_property_range_iterator(
            p, ns, query_spec) for p, ns in zip(p_ranges, ns_ranges)]
    return [cls(i) for i in iters]

  @classmethod
  def validate(cls, job_config):
    """Inherit docs."""
    super(ModelDatastoreInputReader, cls).validate(job_config)
    params = job_config.input_reader_params
    entity_kind = params[cls.ENTITY_KIND_PARAM]
    # Fail fast if Model cannot be located.
    try:
      model_class = util.for_name(entity_kind)
    except ImportError, e:
      raise errors.BadReaderParamsError("Bad entity kind: %s" % e)
    if cls.FILTERS_PARAM in params:
      filters = params[cls.FILTERS_PARAM]
      if issubclass(model_class, db.Model):
        cls._validate_filters(filters, model_class)
      else:
        cls._validate_filters_ndb(filters, model_class)
      property_range.PropertyRange(filters, entity_kind)

  @classmethod
  def _validate_filters(cls, filters, model_class):
    """Validate user supplied filters.

    Validate filters are on existing properties and filter values
    have valid semantics.

    Args:
      filters: user supplied filters. Each filter should be a list or tuple of
        format (<property_name_as_str>, <query_operator_as_str>,
        <value_of_certain_type>). Value type is up to the property's type.
      model_class: the db.Model class for the entity type to apply filters on.

    Raises:
      BadReaderParamsError: if any filter is invalid in any way.
    """
    if not filters:
      return

    properties = model_class.properties()

    for f in filters:
      prop, _, val = f
      if prop not in properties:
        raise errors.BadReaderParamsError(
            "Property %s is not defined for entity type %s",
            prop, model_class.kind())

      # Validate the value of each filter. We need to know filters have
      # valid value to carry out splits.
      try:
        properties[prop].validate(val)
      except db.BadValueError, e:
        raise errors.BadReaderParamsError(e)

  @classmethod
  # pylint: disable=protected-access
  def _validate_filters_ndb(cls, filters, model_class):
    """Validate ndb.Model filters."""
    if not filters:
      return

    properties = model_class._properties

    for f in filters:
      prop, _, val = f
      if prop not in properties:
        raise errors.BadReaderParamsError(
            "Property %s is not defined for entity type %s",
            prop, model_class._get_kind())

      # Validate the value of each filter. We need to know filters have
      # valid value to carry out splits.
      try:
        properties[prop]._do_validate(val)
      except db.BadValueError, e:
        raise errors.BadReaderParamsError(e)

