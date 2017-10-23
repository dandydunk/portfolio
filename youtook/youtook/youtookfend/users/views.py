from django.shortcuts import render
from django.http import HttpResponse

def login(request):
	return HttpResponse("login ok");
	
def register(request):
	return HttpResponse("register ok");
