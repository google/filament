PROTOC=protoc

PY_PROTOS=dims_pb2.py pinpoint_chrome_health_results_pb2.py pinpoint_results_pb2.py sheriff_pb2.py sheriff_config_pb2.py


all: $(PY_PROTOS) ../tracing/tracing/proto/histogram_pb2.py


# We now depend on the tracing proto being defined.
../tracing/tracing/proto/histogram_pb2.py:
	$(MAKE) -C ../tracing/tracing/proto histogram_pb2.py

.PHONY: clean ../tracing/tracing/proto/histogram_pb2.py

%_pb2.py: dashboard/protobuf/%.proto
	$(PROTOC) --python_out=. $<

clean:
	rm -f $(PY_PROTOS); $(MAKE) -C ../tracing/tracing/proto clean
