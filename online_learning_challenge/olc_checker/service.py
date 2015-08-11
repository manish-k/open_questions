import os
import shutil
import datetime
import subprocess
from flask import Flask, request, jsonify, Response, url_for, render_template
#from flask.ext.uploads import patch_request_class
import logging, json, dicttoxml, time
from logging.handlers import RotatingFileHandler
import traceback 

app = Flask(__name__)
#patch_request_class( app, 32 * 1024 )

UPLOAD_DIR = os.path.expanduser( '~/open_questions/online_learning_challenge/solutions/' )
ALLOWED_EXTENSIONS = [ 'py' ]
EVAL_SCRIPT = os.path.expanduser( '~/open_questions/online_learning_challenge/tests/eval.py' )

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1] in ALLOWED_EXTENSIONS

@app.route("/")
def index():
   return render_template( 'index.html' )

@app.route("/test", methods=['POST'])
def get_results( ):
    email = request.form[ 'email' ]
    codefile = request.files['fileToUpload']
    dir_to_save = UPLOAD_DIR + email + '_' + datetime.datetime.now().strftime("%Y%m%d_%I:%M%p") + '/'
    filename = 'solution.py'
    if not os.path.exists( dir_to_save ):
        os.makedirs( dir_to_save )
    if codefile and allowed_file(codefile.filename):
        codefile.save( dir_to_save + filename )
        shutil.copy( EVAL_SCRIPT, dir_to_save )
        open( dir_to_save + '__init__.py', 'a' ).close( )
        subprocess.Popen( [ 'python', dir_to_save + 'eval.py', email ] )
        return "Thanks! The results will be emailed to you at %s" % email
    else:
        return "Please upload a  valid file"

if __name__ == "__main__":
    app.logger.setLevel(logging.INFO)
    formatter = logging.Formatter("[%(asctime)s] - %(levelname)s - %(message)s")
    handler = RotatingFileHandler('error.log', maxBytes=100000000, backupCount=5)
    handler.setFormatter(formatter)
    app.logger.addHandler(handler)
    log = logging.getLogger('werkzeug')
    log.setLevel(logging.DEBUG)
    log.addHandler(handler)
    app.run(host='0.0.0.0', port=8084)
