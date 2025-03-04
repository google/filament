<!-- Javascript for accordion menus, included by index.php, shared with OpenCL man pages -->
<script type="text/javascript">
<!--
var temp, temp2, cookieArray, cookieArray2, cookieCount;
function initiate(){
  cookieCount=0;
  if(document.cookie){
    cookieArray=document.cookie.split(";");
    cookieArray2=new Array();
    for(i in cookieArray){
      cookieArray2[cookieArray[i].split("=")[0].replace(/ /g,"")]=cookieArray[i].split("=")[1].replace(/ /g,"");
    }
  }
  cookieArray=(document.cookie.indexOf("state=")>=0)?cookieArray2["state"].split(","):new Array();
  temp=document.getElementById("containerul");
  for(var o=0;o<temp.getElementsByTagName("li").length;o++){
    if(temp.getElementsByTagName("li")[o].getElementsByTagName("ul").length>0){
      temp2 = document.createElement("span");
      temp2.className = "symbols";
      temp2.style.backgroundImage = (cookieArray.length>0)?((cookieArray[cookieCount]=="true")?"url(bullets-contract.gif)":"url(bullets-expand.gif)"):"url(bullets-expand.gif)";
      temp2.onmousedown=function(){
        showhide(this.parentNode);
        writeCookie();
      }
      temp.getElementsByTagName("li")[o].insertBefore(temp2,temp.getElementsByTagName("li")[o].firstChild)
      temp.getElementsByTagName("li")[o].getElementsByTagName("ul")[0].style.display = "none";
      if(cookieArray[cookieCount]=="true"){
        showhide(temp.getElementsByTagName("li")[o]);
      }
      cookieCount++;
    }
    else{
      temp2 = document.createElement("span");
      temp2.className = "symbols";
      temp2.style.backgroundImage = "url(bullets-end.gif)";
      temp.getElementsByTagName("li")[o].insertBefore(temp2,temp.getElementsByTagName("li")[o].firstChild);
    }
  }
}

function showhide(el){
  el.getElementsByTagName("ul")[0].style.display=(el.getElementsByTagName("ul")[0].style.display=="block")?"none":"block";
  el.getElementsByTagName("span")[0].style.backgroundImage=(el.getElementsByTagName("ul")[0].style.display=="block")?"url(bullets-contract.gif)":"url(bullets-expand.gif)";
}

function writeCookie(){ // Runs through the menu and puts the "states" of each nested list into an array, the array is then joined together and assigned to a cookie.
  cookieArray=new Array()
  for(var q=0;q<temp.getElementsByTagName("li").length;q++){
    if(temp.getElementsByTagName("li")[q].childNodes.length>0){
      if(temp.getElementsByTagName("li")[q].childNodes[0].nodeName=="SPAN" && temp.getElementsByTagName("li")[q].getElementsByTagName("ul").length>0){
        cookieArray[cookieArray.length]=(temp.getElementsByTagName("li")[q].getElementsByTagName("ul")[0].style.display=="block");
      }
    }
  }
  document.cookie="state="+cookieArray.join(",")+";expires="+new Date(new Date().getTime() + 365*24*60*60*1000).toGMTString();
}
//-->
</script>
