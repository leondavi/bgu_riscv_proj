'''
This is a simple script for sending an email via python.
the main "send email" function excepts the following arguments:
(1) gmail_user	- a dedicated gmail address with purpose of sending mails via python
(2) gmail_pass	- password to the user account
(3) to			- a list of emails to send to
(4) subject 	- The subject of the message
(5) body 		- Here is the body of the message, could be as long as you like.\n new lines work very similar to c lang.
'''
from zipfile import ZipFile
from datetime import datetime , date, time
import os
import time
from csv_mid_processing import *

def send_email(	gmail_user	= 'MyPy', #"@gmail.com" is added later.
				gmail_pass	= 'p@ssword',
				to_list		= ['youremail@gmail.com', 'anotheremail@yahoo.com'],
				subject		= 'default message subject',
				body		= 'default message body',
				filename	= 'send_email.py'):
	
	import smtplib
	from email.mime.text import MIMEText
	from email.mime.multipart import MIMEMultipart
	from email.mime.base import MIMEBase
	from email import encoders
	
	gmail_from = gmail_user + '@gmail.com'
	# the next segment defines the text format for the email message.
	msg = MIMEMultipart()
	msg['To'] = ",".join(to_list)
	msg['From'] = gmail_from
	msg['Subject'] = subject
	msg.attach(MIMEText(body, 'plain'))
		
	# attaching a file
	attachment = open(filename, 'rb')
	part = MIMEBase('application', 'octet-stream')
	part.set_payload((attachment).read())
	encoders.encode_base64(part)
	part.add_header('Content-Disposition', "attachment; filename= "+filename)
	
	msg.attach(part)
	
	email_text = msg.as_string()
	try:
		#open server @port 465 for secure connection:
		server_ssl  = smtplib.SMTP_SSL('smtp.gmail.com', 465)
		#extended hello:
		server_ssl.ehlo()
		# log in to gmail account:
		server_ssl.login(gmail_from, gmail_pass)
		# send email:
		server_ssl.sendmail(gmail_from, to_list, email_text)
		# close server connection:
		server_ssl.close()
		#print ('Email sent to' + to_list)
	except:
		print ('something is wrong...')


def get_all_file_paths(directory): 
	# initializing empty file paths list 
	file_paths = []
	# crawling through directory and subdirectories 
	for root, directories, files in os.walk(directory): 
		for filename in files: 
			if filename in ["hello.log", "send_email.py","my_logs.zip"]:
				continue
		# join the two strings in order to form the full filepath. 
			filepath = os.path.join(root, filename)
	#	print(filepath)
			file_paths.append(filepath) 
	
	# returning all file paths 
	return file_paths


def check_logs(directory, file_dic):
	ff = 0
	for root, directories, files in os.walk(directory): 
		for filename in files: 
			if filename in ["hello.log", "send_email.py","my_logs.zip"]:
				continue
			if filename in file_dic.keys() and (file_dic[filename] == "Done" or file_dic[filename] == "Fail"):
				continue
			with open(filename, 'r') as fp:
				for line in fp:
						if ("Benchmarking Done" in line):
							file_dic[filename] = "Done"
							ff = 1
							break
						elif (("fail" in line) or ("abort" in line) or ("Fail" in line) or ("Abort" in line)):
							file_dic[filename] = "Fail"
							ff = 1
							break
	return ff, file_dic


def convert_bytes(num):
	"""
	this function will convert bytes to MB.... GB... etc
	"""
	for x in ['bytes', 'KB', 'MB', 'GB', 'TB']:
		if num < 1024.0:
			return (num, x)
		num /= 1024.0


def paramFailAdd(cmd, paramList):
	tmp = cmd
	val = ""
	while len(tmp) > 0:
		start = tmp.find("=") +1
		end = tmp.find(" ",start)
		if '/' in tmp[start:end]:
			tmp = tmp[start +3:]
		elif tmp[start:end].isdigit():
			val += ", " + tmp[start:end]
		tmp = tmp[start +3:]
	if val not in paramList:
		paramList[val] = cmd[-25:]
	else:
		paramList[val] += cmd[-25:] + ' '
	return paramList


def checkFails(statDir, jobTracker, failLog):
	out_str = "--outdir="
	cmd_str = "cmd:"
	workloads = []
	failLog_sum = "failSum.log"
	paramDic = {"val": workloads}
	with open(os.path.join(statDir,failLog),'w') as fl:
		with open(os.path.join(statDir,jobTracker), 'r') as jt:
			for line in jt:
				start = line.find(out_str)+len(out_str)
				end = line.find(" ",start)
				outdir = line[start:end]
				# print("outdir {}\n{}\n{}\n".format(outdir, start, end)) #debug
				stats_file = os.path.join(outdir,"stats.txt")
				size,xx = convert_bytes(os.stat(stats_file).st_size)
				if size == 0:
					start = line.find(cmd_str)+len(cmd_str) +1
					cmd = line[start:]
					fl.write(cmd + "\n")
					paramFail = paramFailAdd(cmd, paramDic)
	with open(os.path.join(statDir,failLog_sum),'w') as fls:    
		for key in paramFail:
			fls.write(key[2:] + ":\t" + str(paramFail[key]) + "\n")

def main():
	COMP_NAME = "SHLOMO5@GBU_LAB"
	file_dic = {"file_name" : "flag"}
	flag = 1
	first_flag = 1
	prev_size = 0
	failLog = "failed.log"
	jobFile = "job_tracker.txt"
	stat_path = "./statistics/"
	cvs_path = "./csvs/out/"
	while (flag == 1):
	    now = datetime.now()
	    if first_flag == 1:
		first_flag = 0
	    elif ((6 != now.hour) and (12 != now.hour) and (18 != now.hour)):
		continue
	    jj,ee = cvs_update(stat_path)
            file_paths = []
	    for csv_file in os.listdir(cvs_path):
	        file_paths.append(os.path.join(cvs_path, csv_file))
	    zipfilename = "my_logs1.zip"
	    with ZipFile(zipfilename,'w') as zip: 
	# writing each file one by one 
		for file in file_paths:
	            zip.write(file)
	    title = 'Updated logfile!'
	    text = 'from: '+ COMP_NAME +'\nAttached are updated logfiles from now {}.\n'.format(now.strftime("%d/%m/%Y %H:%M:%S"))
	    size,xx = convert_bytes(os.stat(zipfilename).st_size)
	    text = text + "\nLog zipfile size: {}{}\nJobs Passed: {}\nJobs Failed: {}".format(size, xx, jj, ee)
		#if ff == 1:
			#text = text + 'some files flaged exceptions: {}'.format(file_dic)
            print ("sent email with text:\n{}\n".format(text))
	    file_to_send = zipfilename
	    send_email('my.matlab.acnt', 'mymatlabaccount', ['ohadbox@post.bgu.ac.il'], title, text, zipfilename)
            time.sleep(3600)

if __name__ == '__main__':
	main()

