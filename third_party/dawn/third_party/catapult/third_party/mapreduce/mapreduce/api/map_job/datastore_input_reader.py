#!/usr/bin/env python
"""Datastore Input Reader implementation for the map_job API."""
import logging

from mapreduce import datastore_range_iterators as db_iters
from mapreduce import errors
from mapreduce.api.map_job import abstract_datastore_input_reader

# pylint: disable=invalid-name


class DatastoreInputReader(abstract_datastore_input_reader
                           .AbstractDatastoreInputReader):
  """Iterates over an entity kind and yields datastore.Entity."""

  _KEY_RANGE_ITER_CLS = db_iters.KeyRangeEntityIterator

  @classmethod
  def validate(cls, job_config):
    """Inherit docs."""
    super(DatastoreInputReader, cls).validate(job_config)
    params = job_config.input_reader_params
    entity_kind = params[cls.ENTITY_KIND_PARAM]
    # Check for a "." in the entity kind.
    if "." in entity_kind:
      logging.warning(
          ". detected in entity kind %s specified for reader %s."
          "Assuming entity kind contains the dot.",
          entity_kind, cls.__name__)
    # Validate the filters parameters.
    if cls.FILTERS_PARAM in params:
      filters = params[cls.FILTERS_PARAM]
      for f in filters:
        if f[1] != "=":
          raise errors.BadReaderParamsError(
              "Only equality filters are supported: %s", f)
