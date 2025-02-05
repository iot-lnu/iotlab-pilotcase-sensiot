# server.py

from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/api/endpoint', methods=['POST'])
def handle_post():
    data = request.get_json()
    total_people = data.get('total_people', None)
    
    if total_people is not None:
        print(f"Received total_people: {total_people}")
        return jsonify({"message": "Data received", "total_people": total_people}), 200
    else:
        return jsonify({"error": "Invalid data"}), 400

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8888)

