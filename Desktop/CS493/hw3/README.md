# BuildRestAPI

Purpose
In this course, you are learning to design, implement, and test REST APIs that are deployed on the Cloud.

In this assignment, you are being provided the design specification of a REST API and a test suite. The focus of this assignment is thus on implementing a REST API and deploying the service on Google App Engine. By the end of the assignment you will have a live REST API service on the cloud.

Instructions
Your REST service models a simple marina.

There are two types of resources, boats and slips (slips are "parking spots" for boats).
A boat is either at sea or it is assigned to a slip.
A slip has space for only one boat at a time. At any given time, a slip is
Either empty, when there is no boat at the slip, or
Occupied, when there is a boat in it.
There is no permanent assignment between a boat and a slip. As an example, the following sequence of events can happen
Boat A is in Slip 1.
Boat A departs and is at sea.
Boat B arrives at the marina and is now in Slip 1.
Boat A arrives at the marina and is now in Slip 4.
Data Model
In this assignment, you will use Cloud Datastore to store your data. There are two kinds of entities in your data model.

Boat
Property Name	Data type	Notes
id	Integer	
The  id of the boat. Datastore automatically generates it.

Don't add it yourself as a property of the entity.

name	String	Name of the boat.
type	String	
Type of the boat. E.g., Sailboat, Catamaran, etc. 

length	Integer	The length of the boat in feet
Slip
Property Name	Data Type	Notes
id	Integer	The id of the slip. Datastore automatically generates it.
Don't add it yourself as a property of the entity.

number	Integer	The number of the slip.
current_boat	Integer	If there is a boat at the slip, then id of that boat. Otherwise null.
Required Functionality
Here is a description of the functionality. Note that direct modification of a slip is not allowed.

Add a boat.
All newly created boats should start "at sea" and not in a slip.
View a boat.
You can view the details of a single boat.
View all boats.
You can view the details of all boats.
Modify a boat.
You can modify any property of a boat other than the id.
Delete a boat.
If the boat being deleted is in a slip, deleting the boat should automatically make the slip empty.
Add a slip.
All newly created slips should be empty.
View a slip.
You can view the details of a slip.
View all slips
You can view the details of all slips.
Delete a slip.
When a slip is deleted, any boat that was occupying that slip is now considered "at sea."
A boat's arrival at a slip.
A boat should be assigned to this slip if this slip is empty
If the slip is occupied, the service will return an error.
A boat's departure from a slip.
The slip now becomes empty.
REST API Documentation
Here is the doc for the REST API you need to implement. This API models the functionality described above. The REST API doc includes

Details of the endpoints (i.e., URLs) you need to implement
Details of the format of requests and responses for each endpoint
Errors your code needs to catch, and
Status codes that must be returned in case of success and in case of errors.
Testing the API
You can test your API by downloading the following  two JSON files, one with a Postman Collection and the other with a Postman Environment.

1. Postman CollectionDownload Postman Collection

These tests cover the functionality your API should implement.
We are going to use a similar test suite for grading the assignment. I am saying similar, rather than same, for the following reasons
The provided test suite creates 2 boats and 1 slip. The grading suite may create more boats and more slips.
The grading suite may use different name, length, types values for boats, and different values for the number attribute of slips it creates.
When we run the test collection we will add a delay of 3 seconds between the tests (Exploration - Postman Tips describes how to do that).
Overall the point of using a different suite for grading is that students must not code to the actual attribute values or operations in the suite.
However, if your implementation works correctly with this provided test suite and doesn't have any code which is specifically tailored to the actual attribute values of boats and slips in the test suite, it should work correctly with the grading suite.
2. Postman Environment Download Postman Environment

The environment contains 3 pre-defined variables
app_url: This allows you to easily run the test suite against the service running on your computer or running it against the service deployed on GCP.
To run against service on your computer, set it to http://localhost:8080Links to an external site.
To run against service deployed on GCP, set it to the URL your project is deployed to, e.g., https://your-project-name.region.appspot.comLinks to an external site.
invalid_boat_id: This allows the suite to run tests against boats that don't exist. I have set its value to 1 and that should work for you as well.
invalid_slip_id: Similar to above, to allow running tests against slips that don't exist.
Submission Details
You can use either Node.js or Python 3 to implement the API, which must be deployed to Google App Engine.  Any exceptions to this must have been pre-approved by the instructor as discussed in relevant Ed Discussion Board threads.

You need to submit two files

1. A file youronid_marina.pdf that should contain two things;

1. The link to your working REST service deployed on GCP. If the link you submit has "localhost" in it your project isn't on the cloud and will not receive any points.

2. A short (half to one page) critique of the following 3 endpoints (endpoint = HTTP method + URL)

Modify a boat
A boat arrives at a slip
A boat departs from a slip
In your critique evaluate the design of these three endpoint.
Is the chosen HTTP method/verb appropriate for the functionality? Or do you think a different HTTP method is more appropriate?
Does the URL pattern follows RESTful principles? If not, what would a better URL pattern?
Note you need to provide arguments for your answer.
2. A file youronid_marina.zip that should include all the source code for your project. You must not include node_modules or Python env in the zip file.

Grading Criteria
Each category on the rubric will be graded on a range so there are partial points. 

Hints
In Module 3, the exploration "Google App Engine and Node.js" has code for a REST API implemented in Node.js. The code consists of just 3 files. If you are using Node.js,  you are free to copy these files and start modifying them for your implementation.

Similarly, the exploration "Google App Engine and Python" has code for a REST API implemented in Python. The code consists of 4 files. If you are using Python, you are free to use these files to start your implementation.