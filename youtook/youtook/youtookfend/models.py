from django.db import models

class Users(models.Model):
	email = models.CharField(max_length=255)
	userName = models.CharField(max_length=255)
	password = models.CharField(max_length=255)
	joinDate = models.DateTimeField()