from flask import Flask, request, jsonify
from flask_httpauth import HTTPBasicAuth
import mysql.connector
from mysql.connector import Error

app = Flask(__name__)
auth = HTTPBasicAuth()

# Define a dictionary of allowed username and tokens (used as passwords)
USER_DATA = {
    "admin": "secret_stuff"
}

@auth.verify_password
def verify_password(username, password):
    if username in USER_DATA and USER_DATA[username] == password:
        return username
    return None

def insert_plate_count(plates, uid):
    try:
        connection = mysql.connector.connect(
            host='db',           # Correct container service name as per Docker setup
            database='mydatabase',
            user='myuser',
            password='mypassword'
        )

        cursor = connection.cursor()

        # Modified query to insert both count and uid.
        insert_query = "INSERT INTO guest_counts (count, uid) VALUES (%s, %s)"
        cursor.execute(insert_query, (plates, uid))
        connection.commit()

        print(f"Inserted record with count: {plates} and uid: {uid}")

    except Error as e:
        print("Error while connecting to MySQL", e)
    finally:
        if connection.is_connected():
            cursor.close()
            connection.close()

@app.route('/listen', methods=['POST'])
@auth.login_required
def listen():
    # For debugging: Print the raw Authorization header
    raw_auth = request.headers.get("Authorization")
    print("Raw Authorization header:", raw_auth)

    data = request.json
    plates = data.get('plates')  # Extract 'plates' from the incoming JSON
    uid = data.get('uid')        # Extract 'uid' from the incoming JSON

    if plates is not None:
        insert_plate_count(plates, uid)  # Insert value into the database

    return jsonify({
        "message": "POST request received successfully",
        "received_data": data
    }), 200

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=80)
