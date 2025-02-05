# Use the official lightweight Python image.
FROM python:3.9-slim

# Set the working directory in the container
WORKDIR /app

# Copy requirements.txt and the server script to the working directory
COPY server.py /app/server.py

# Install Flask
RUN pip install Flask

# Expose port 8888 to the outside world
EXPOSE 8888

# Command to run the Flask app
CMD ["python", "server.py"]

