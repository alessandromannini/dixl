"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from flask import Flask

from flask_migrate import Migrate
from flask_sqlalchemy import SQLAlchemy

from config import Config

# Application main object
app = Flask(__name__)
app.config.from_object(Config)

# DB Object
db = SQLAlchemy(app)
migrate = Migrate(app, db)

# SQLite specific restictions
with app.app_context():
    if db.engine.url.drivername == 'sqlite':
        migrate.init_app(app, db, render_as_batch=True)
    else:
        migrate.init_app(app, db)

from application import models, routes