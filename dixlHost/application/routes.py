"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from flask import render_template
from application import app

@app.route("/")
def homepage():
    return render_template("home.html")

@app.route("/about")
def about():
    return render_template("about.html")
