# NOTE: For this makefile to work, you *must* have the following cloned into ../../.. :
# - https://github.com/googleapis/googleapis
# - https://skia.googlesource.com/buildbot.git

generate_pb_grpc:
	python3 -m grpc_tools.protoc \
	-I ../../../skia/buildbot \
	-I ../../../googleapis \
	--python_out=. \
	--grpc_python_out=. \
	cabe/proto/v1/service.proto

generate_spec_pb:
	python3 -m grpc_tools.protoc \
	-I ../../../skia/buildbot/ \
	--python_out=. \
	cabe/proto/v1/spec.proto

generate_analysis_pb:
	python3 -m grpc_tools.protoc \
	-I ../../../skia/buildbot \
	--python_out=. \
	cabe/proto/v1/analysis.proto

generate_pb: generate_spec_pb generate_analysis_pb

generate: generate_pb generate_pb_grpc

deploy_staging:
	gcloud builds submit \
	--region=us-central1 \
	--config=cloudbuild.yaml \
	--substitutions=_VERSION=$(USER) \
	--project=chromeperf .
