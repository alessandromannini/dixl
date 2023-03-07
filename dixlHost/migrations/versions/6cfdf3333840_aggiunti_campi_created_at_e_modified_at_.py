"""Aggiunti campi created_at e modified_at a tutte le tabelle

Revision ID: 6cfdf3333840
Revises: 
Create Date: 2023-02-28 21:34:47.232320

"""
from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision = '6cfdf3333840'
down_revision = None
branch_labels = None
depends_on = None


def upgrade():
    # ### commands auto generated by Alembic - please adjust! ###
    op.create_table('layout',
    sa.Column('id', sa.Integer(), nullable=False),
    sa.Column('name', sa.String(length=50), nullable=False),
    sa.Column('gridRows', sa.Integer(), nullable=False),
    sa.Column('gridCols', sa.Integer(), nullable=False),
    sa.Column('created_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.Column('modified_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.PrimaryKeyConstraint('id')
    )
    op.create_table('user',
    sa.Column('id', sa.Integer(), nullable=False),
    sa.Column('username', sa.String(length=50), nullable=False),
    sa.Column('password', sa.String(length=50), nullable=False),
    sa.Column('created_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.Column('modified_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.PrimaryKeyConstraint('id'),
    sa.UniqueConstraint('username')
    )
    op.create_table('node',
    sa.Column('id', sa.Integer(), nullable=False),
    sa.Column('name', sa.String(length=50), nullable=False),
    sa.Column('type', sa.String(length=2), nullable=False),
    sa.Column('layout_id', sa.Integer(), nullable=True),
    sa.Column('gridRow', sa.Integer(), nullable=False),
    sa.Column('gridCol', sa.Integer(), nullable=False),
    sa.Column('orientation', sa.String(length=2), nullable=False),
    sa.Column('created_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.Column('modified_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.ForeignKeyConstraint(['layout_id'], ['layout.id'], ),
    sa.PrimaryKeyConstraint('id')
    )
    op.create_table('route',
    sa.Column('id', sa.Integer(), nullable=False),
    sa.Column('name', sa.String(length=50), nullable=False),
    sa.Column('layout_id', sa.Integer(), nullable=True),
    sa.Column('created_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.Column('modified_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.ForeignKeyConstraint(['layout_id'], ['layout.id'], ),
    sa.PrimaryKeyConstraint('id')
    )
    op.create_table('route_list',
    sa.Column('route_id', sa.Integer(), nullable=False),
    sa.Column('index', sa.Integer(), nullable=False),
    sa.Column('node_id', sa.Integer(), nullable=True),
    sa.Column('created_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.Column('modified_at', sa.DateTime(), server_default=sa.text('(CURRENT_TIMESTAMP)'), nullable=False),
    sa.ForeignKeyConstraint(['node_id'], ['node.id'], ),
    sa.ForeignKeyConstraint(['route_id'], ['route.id'], ),
    sa.PrimaryKeyConstraint('route_id', 'index')
    )
    # ### end Alembic commands ###


def downgrade():
    # ### commands auto generated by Alembic - please adjust! ###
    op.drop_table('route_list')
    op.drop_table('route')
    op.drop_table('node')
    op.drop_table('user')
    op.drop_table('layout')
    # ### end Alembic commands ###
