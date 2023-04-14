"""
@author         : "Alessandro Mannini"
@organization   : "Università degli Studi di Firenze"
@contact        : "alessandro.mannini@gmail.com"
@date           : "Jan 10, 2023"
@version        : "1.0.0"
"""
from datetime import datetime
from typing import Annotated, List

from application import db

class User(db.Model):
    __tablename__ = "user"

    id: db.Mapped[int] = db.mapped_column(db.Integer, primary_key=True)
    username: db.Mapped[str] = db.mapped_column(db.String(50), unique=True)
    password: db.Mapped[str] = db.mapped_column(db.String(50))
    created_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())
    modified_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())

class Layout(db.Model):
    __tablename__ = "layout"

    id: db.Mapped[int] = db.mapped_column(db.Integer, primary_key=True)
    name: db.Mapped[str] = db.mapped_column(db.String(50))
    nodes: db.Mapped[List["Node"]] = db.relationship(back_populates="layout")
    routes: db.Mapped[List["Route"]] = db.relationship(back_populates="layout")
    gridRows: db.Mapped[int] = db.mapped_column(db.Integer, default=0)
    gridCols: db.Mapped[int] = db.mapped_column(db.Integer, default=0)
    created_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())
    modified_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())

    def updateGridSize(self):
        for node in self.nodes:
            if node.gridRow > self.gridRows:
                self.gridRows = node.gridRow
            if node.gridCol > self.gridCols:
                self.gridCols = node.gridCol

class Node(db.Model):
    __tablename__ = "node"

    id: db.Mapped[int]  = db.mapped_column(db.Integer, primary_key=True)
    name: db.Mapped[str] = db.mapped_column(db.String(50))
    type: db.Mapped[str]  = db.mapped_column(db.String(2))
    layout_id = db.mapped_column(db.Integer, db.ForeignKey('layout.id'))
    layout: db.Mapped["Layout"] = db.relationship(back_populates="nodes")
    gridRow: db.Mapped[int] = db.mapped_column(db.Integer, default=-1)
    gridCol: db.Mapped[int] = db.mapped_column(db.Integer, default=-1)
    orientation: db.Mapped[str] = db.mapped_column(db.String(2))
    created_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())
    modified_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())

class Route(db.Model):
    __tablename__ = "route"

    id: db.Mapped[int]  = db.mapped_column(db.Integer, primary_key=True)
    name: db.Mapped[str] = db.mapped_column(db.String(50))
    layout_id = db.mapped_column(db.Integer, db.ForeignKey('layout.id'))
    layout: db.Mapped["Layout"] = db.relationship(back_populates="routes")
    nodes: db.Mapped[List["RouteList"]] = db.relationship(back_populates="route")
    created_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())
    modified_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())

class RouteList(db.Model):
    __tablename__ = "route_list"

    route_id: db.Mapped[int]  = db.mapped_column(db.Integer, db.ForeignKey('route.id'), primary_key=True)
    route: db.Mapped["Route"] = db.relationship(back_populates="nodes")
    index: db.Mapped[int] = db.mapped_column(db.Integer, primary_key=True)
    node_id = db.mapped_column(db.Integer, db.ForeignKey('node.id'))
    created_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())
    modified_at: db.Mapped[datetime] = db.mapped_column(db.DateTime, server_default=db.func.CURRENT_TIMESTAMP())
