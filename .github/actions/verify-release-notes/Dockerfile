FROM python:3.12.13

RUN pip3 install pygithub==1.55

ENV PYTHONUNBUFFERED=1

COPY verify_release_notes.py /verify_release_notes.py

RUN chmod +x /verify_release_notes.py
ENTRYPOINT [ "/verify_release_notes.py" ]
