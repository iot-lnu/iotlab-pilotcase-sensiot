FROM python:3.13-slim
WORKDIR /app
COPY server.py /app/server.py
RUN pip install Flask
EXPOSE 8888
CMD ["python", "server.py"]

