
describe("error report", function() {

   it('should parse json if json is given as the body', function(){
      var jsonFromServer = {something:'went wrong'};
   
      var report = errorReport(
         0, 
         JSON.stringify(jsonFromServer));          
   
      expect(report.jsonBody).toEqual(jsonFromServer);
   });
   
   it('should not have jsonBody if no body is given', function(){   
      var report = errorReport();          
   
      expect(report.jsonBody).toBeUndefined();      
   });
   
   it('should not have jsonBody if body is given but it is not json', function(){

      var responseFromServer = "<html>blah blah</html>";
   
      var report = errorReport(404, responseFromServer);          
   
      expect(report.jsonBody).toBeUndefined();      
      expect(report.body).toBe(responseFromServer);      
   });
   
   it('should store status code', function(){   
      var report = errorReport(404);          
   
      expect(report.statusCode).toBe(404);      
   });
   
   it('should store thrown thing', function(){
      var thrown = new Error('something bad happened');
      
      var report = errorReport(404, '', thrown);          
   
      expect(report.thrown).toBe(thrown);      
   });         
   
});