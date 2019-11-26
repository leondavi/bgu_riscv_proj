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

def send_email(	gmail_user	= 'MyPy', #"@gmail.com" is added later.
		gmail_pass	= 'p@ssword',
		to_list		= ['youremail@gmail.com', 'anotheremail@yahoo.com'],
		subject		= 'default message subject',
		body		= 'default message body',
                filename        = 'send_email.py'):

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

def main():
    # change from datetime.hour to date.day
#    now = datetime.now()
#    hour = now.hour
    file_dic = {"file_name" : "flag"}
    flag = 1
    first_flag = 1
    #print(prev_hour)
    while (flag == 1):
    #    ff,file_dic = check_logs("./", file_dic)
        now = datetime.now()
        if first_flag == 1:
            first_flag = 0
        elif ((8 != now.hour) and (20 != now.hour)):
            continue
        #else:
        #    prev_hour = now.hour
        # zip log files:
        file_paths = get_all_file_paths("~/workspace/bgu_riscv_proj/gem5/statistics/")
      #  file_paths = get_all_file_paths("./")
        zipfilename = "my_logs.zip"
        with ZipFile(zipfilename,'w') as zip: 
        # writing each file one by one 
            for file in file_paths:
                zip.write(file)
        title = 'Updated logfiles!'
        text = 'Attached are updated logfiles from now {}.\n'.format(now.strftime("%d/%m/%Y %H:%M:%S"))
#        if ff == 1:
#            text = text + 'some files flaged exceptions: {}'.format(file_dic)
        print ("sent email with text:{}".format(text))
        # send_email('my.matlab.acnt', 'mymatlabaccount', ['awadob@gmail.com','bshlomo@gmail.com'], title, text)
        send_email('my.matlab.acnt', 'mymatlabaccount', ['ohadbox@post.bgu.ac.il'], title, text, zipfilename)
        time.sleep(3600)
if __name__ == '__main__':
	main()

