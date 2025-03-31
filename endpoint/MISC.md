sudo docker build -t flask-listener .
sudo docker run -p 80:80 flask-listener

CREATE TABLE guest_counts (
    id INT AUTO_INCREMENT PRIMARY KEY,
    count INT NOT NULL,
    uid VARCHAR(64),
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
