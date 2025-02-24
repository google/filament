from flask import Flask, jsonify, send_from_directory
from glob import glob

app = Flask(__name__)


@app.route('/raw/<path:path>')
def read_file(path):
    return send_from_directory('../char-dataset', path, as_attachment=True), 200, {"Content-Type": "text/plain"}


@app.route("/")
def read_targets():
    return jsonify(
        [
            el.replace("./char-dataset", "/raw").replace("\\", "/") for el in sorted(glob("./char-dataset/**/*"))
        ]
    )


@app.route("/edge/empty/plain")
def read_empty_response_plain():
    return b"", 200, {"Content-Type": "text/plain"}


@app.route("/edge/empty/json")
def read_empty_response_json():
    return b"{}", 200, {"Content-Type": "application/json"}


@app.route("/edge/gb18030/json")
def read_gb18030_response_json():
    return '{"abc": "我没有埋怨，磋砣的只是一些时间。。今觀俗士之論也，以族舉德，以位命賢，茲可謂得論之一體矣，而未獲至論之淑真也。"}'.encode("gb18030"), 200, {"Content-Type": "application/json"}


if __name__ == "__main__":
    app.run(host="127.0.0.1", port=8080)
