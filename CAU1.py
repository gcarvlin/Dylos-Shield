import csv, datetime, re, subprocess, json, os.path

# Initialize variables
#
url="MYURL"
cert="/etc/ssl/certs/cacert4.pem"
auth="USERNAME:PASS"
column_totals=[0,0,0,0,0,0]
column_avgs=[0,0,0,0,0,0]
counter = 0
dylos_name = ""

#Clean Data
with open("/root/temp_data.txt", "r") as f:
	with open("/root/temp_data_clean.txt", "w") as f2:
		p = re.compile('\w+,\d{10,10},\d{2,2}/\d{2,2}/\d{2,2} \d{2,2}:\d{2,2}:\d{2,2},\d+,\d+,\d+,\d+,\d+.\d+,\d+.\d+')
		for line in f:
			if p.match(line)!=None:
				f2.writelines(line)

#Average Data
with open("/root/temp_data_clean.txt") as f:
	reader = csv.reader(f)
	for row in reader:
		if counter==0:
			dylos_name = row[0]
		for n in range(0,6):
			column_totals[n] += float(row[n+3])
		counter += 1
	for n in range(0,6):
		column_avgs[n] = column_totals[n]/counter

##Upload Data
# Functions
#
timestamp = 1000*int((datetime.datetime.utcnow()-datetime.datetime(1970,1,1)).total_seconds())

def prepdata(value, tags):
	"This prepares a data structure to send in JSON format"
	data = {
				"name" : dylos_name, 
				"timestamp" : timestamp, 
				"value": value, 
				"tags" : tags
			}
	return [data]

def prepbin(n,bin_data):
	"This prepares data for a bin."
	return prepdata(bin_data, { "Bin" : "bin" + str(n) })

# Prepare the bin data
#
bins = []
for n in range(0, 4):
	bins.append(prepbin(n+1,column_avgs[n]))

# Prepare temperature and relative humidity data
#
temp = prepdata(column_avgs[5], { "Temp" : "temp" })
rh = prepdata(column_avgs[4], { "RH" : "rh" })

# Send the data with curl
#
items = bins + [temp, rh]
for item in items:
	print json.dumps(item)
	subprocess.call(["curl",url,"-u",auth,"-d",json.dumps(item),"--cacert",cert])
	print timestamp


#copy file to permanent position, if this is the first time write the header row
if not os.path.isfile("/mnt/sda1/data.csv"):
	f = open("/mnt/sda1/data.csv", "w")
	f.writelines("DylosName,Timestamp,DylosTimestamp,Bin1,Bin2,Bin3,Bin4,RH,Temp")
with open("/mnt/sda1/data.csv", "a") as f:
	with open("temp_data_clean.txt", "r") as f2:
		f.write(f2.read())