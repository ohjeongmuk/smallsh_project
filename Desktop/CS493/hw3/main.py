from google.cloud import datastore
from flask import Flask, request
import json
import constants

project_id = "assignment3-402621"


app = Flask(__name__)
client = datastore.Client(project = project_id)



@app.route('/')
def index():
    return "Please navigate to /lodgings to use this API"\

@app.route('/lodgings', methods=['POST','GET'])
def lodgings_get_post():
    if request.method == 'POST':
        content = request.get_json()
        new_lodging = datastore.entity.Entity(key=client.key(constants.lodgings))
        new_lodging.update({"name": content["name"], "description": content["description"],
          "price": content["price"]})
        client.put(new_lodging)
        return str(new_lodging.key.id)
    elif request.method == 'GET':
        query = client.query(kind=constants.lodgings)
        results = list(query.fetch())
        for e in results:
            e["id"] = e.key.id
        return json.dumps(results)
    else:
        return 'Method not recognized'

@app.route('/boats', methods = ['POST'])
def creating_boats_post():
    content = request.get_json()
    new_boat = datastore.entity.Entity(key=client.key(constants.boats))
    
    print(type(content))
    for e in content:
        print(e)
    return ('', 400)

@app.route('/lodgings/<id>', methods=['PUT','DELETE','GET'])
def lodgings_put_delete(id):
    if request.method == 'PUT':
        content = request.get_json()
        lodging_key = client.key(constants.lodgings, int(id))
        lodging = client.get(key=lodging_key)
        lodging.update({"name": content["name"], "description": content["description"],
          "price": content["price"]})
        client.put(lodging)
        return ('',200)
    elif request.method == 'DELETE':
        key = client.key(constants.lodgings, int(id))
        client.delete(key)
        return ('',200)
    elif request.method == 'GET':
        lodging_key = client.key(constants.lodgings, int(id))
        lodging = client.get(key=lodging_key)
        return json.dumps(lodging)
    else:
        return 'Method not recognized'

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8080, debug=True)