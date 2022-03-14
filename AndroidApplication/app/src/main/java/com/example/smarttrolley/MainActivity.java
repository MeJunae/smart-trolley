package com.example.smarttrolley;

import android.annotation.SuppressLint;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.net.http.SslError;
import android.os.Bundle;
import android.view.View;
import android.webkit.SslErrorHandler;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import java.math.BigDecimal;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class MainActivity extends AppCompatActivity {

    private EditText etIpAddress, etProportional, etIntegral, etDerivative, etManualPath, etProportionality;
    private Button btnStart, buttonA, buttonB, buttonC, buttonD, buttonE, buttonF, buttonG, buttonH, buttonI, buttonJ, buttonK, buttonL, buttonM, buttonN, buttonO, buttonP;
    private Button buttonPositionA, buttonPositionB, buttonPositionC, buttonPositionD, buttonPositionE, buttonPositionF, buttonPositionG, buttonPositionH, buttonPositionI, buttonPositionJ, buttonPositionK, buttonPositionL, buttonPositionM, buttonPositionN, buttonPositionO, buttonPositionP;
    private WebView webView;
    private String ip_address;
    private TextView errorTV, tvProportionality;
    private BigDecimal proportional, integral, derivative, proportionality;
    private RadioButton north, east, south, west, basic, pid, fuzzyPid;
    private TextView tvPid, tvProportional, tvIntegral, tvDerivative;
    private char lastSelectedPosition;
    private RadioGroup rgController;
    private String path;

    @SuppressLint("SetJavaScriptEnabled")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        etIpAddress = findViewById(R.id.editText_ipaddress);
        etProportional = findViewById(R.id.editText_proportional);
        etIntegral = findViewById(R.id.editText_integral);
        etDerivative = findViewById(R.id.editText_derivative);
        etManualPath = findViewById(R.id.editText_manualpath);
        errorTV = findViewById(R.id.textview_error);

        btnStart = findViewById(R.id.start_button);
        buttonA = findViewById(R.id.buttonA);
        buttonB = findViewById(R.id.buttonB);
        buttonC = findViewById(R.id.buttonC);
        buttonD = findViewById(R.id.buttonD);
        buttonE = findViewById(R.id.buttonE);
        buttonF = findViewById(R.id.buttonF);
        buttonG = findViewById(R.id.buttonG);
        buttonH = findViewById(R.id.buttonH);
        buttonI = findViewById(R.id.buttonI);
        buttonJ = findViewById(R.id.buttonJ);
        buttonK = findViewById(R.id.buttonK);
        buttonL = findViewById(R.id.buttonL);
        buttonM = findViewById(R.id.buttonM);
        buttonN = findViewById(R.id.buttonN);
        buttonO = findViewById(R.id.buttonO);
        buttonP = findViewById(R.id.buttonP);
        buttonPositionA = findViewById(R.id.buttonPositionA);
        buttonPositionB = findViewById(R.id.buttonPositionB);
        buttonPositionC = findViewById(R.id.buttonPositionC);
        buttonPositionD = findViewById(R.id.buttonPositionD);
        buttonPositionE = findViewById(R.id.buttonPositionE);
        buttonPositionF = findViewById(R.id.buttonPositionF);
        buttonPositionG = findViewById(R.id.buttonPositionG);
        buttonPositionH = findViewById(R.id.buttonPositionH);
        buttonPositionI = findViewById(R.id.buttonPositionI);
        buttonPositionJ = findViewById(R.id.buttonPositionJ);
        buttonPositionK = findViewById(R.id.buttonPositionK);
        buttonPositionL = findViewById(R.id.buttonPositionL);
        buttonPositionM = findViewById(R.id.buttonPositionM);
        buttonPositionN = findViewById(R.id.buttonPositionN);
        buttonPositionO = findViewById(R.id.buttonPositionO);
        buttonPositionP = findViewById(R.id.buttonPositionP);
        north = findViewById(R.id.north);
        east = findViewById(R.id.east);
        south = findViewById(R.id.south);
        west = findViewById(R.id.west);
        tvPid = findViewById(R.id.textview_pid);
        tvProportional = findViewById(R.id.textview_proportional);
        tvIntegral = findViewById(R.id.textview_integral);
        tvDerivative = findViewById(R.id.textview_derivative);
        rgController = findViewById(R.id.radiogroup_algo);
        basic = findViewById(R.id.withoutfuzzypid);
        pid = findViewById(R.id.withpid);
        fuzzyPid = findViewById(R.id.withfuzzy);
        tvProportionality = findViewById(R.id.textview_p);
        etProportionality = findViewById(R.id.editText_proportionality);

        webView = findViewById(R.id.webview);
        webView.getSettings().setJavaScriptEnabled(true);
        webView.getSettings().setBuiltInZoomControls(true);
        webView.setWebViewClient(new WebViewClient(){
            @Override
            public void onReceivedSslError(WebView view, SslErrorHandler handler, SslError error) {
                super.onReceivedSslError(view, handler, error);
            }
        });

        autofill();
    }

    @Override
    protected void onStart() {
        super.onStart();
        this.lastSelectedPosition = '-';
        fuzzyPid.setChecked(true);
        setVisiblePID(false);
        setVisibleP(false);
        north.setChecked(true);
        rgController.setOnCheckedChangeListener((group, checkedId) -> {
            if(checkedId == R.id.withpid){
                setVisiblePID(true);
                setVisibleP(false);
            }else if(checkedId == R.id.withoutfuzzypid){
                setVisiblePID(false);
                setVisibleP(true);
            }else{
                setVisiblePID(false);
                setVisibleP(false);
            }
        });
        btnStart.setOnClickListener(v -> {
            setErrorTV(false, "");
            if(!checkValidIPAddress(etIpAddress.getText().toString())){
                setErrorTV(true, "Invalid IP Address");
                return;
            }
            if(pid.isChecked() && !checkPIDVar()){
                return;
            }
            if(basic.isChecked() && !checkProportionality()){
                return;
            }
            ip_address = etIpAddress.getText().toString();
            String url = "http://" + ip_address + "/";
            if(etManualPath.getText().toString().equals("")) {
                setErrorTV(true, "Manual Path is empty");
            }else if(!etManualPath.getText().toString().equals("")){
                this.path = etManualPath.getText().toString();
            }else{
                setErrorTV(true, "Invalid!");
                return;
            }

            if(fuzzyPid.isChecked()){
                url = url + "fuzzy?path=" + this.path;
            }else if(basic.isChecked()){
                url = url + "basic?path=" + this.path + "&proportionality=" + this.proportionality;
            }else{
                url = url + "pid?path=" + this.path +
                    "&kp=" + this.proportional  + "&" +
                    "ki=" + this.integral  + "&" +
                    "kd=" + this.derivative;
            }
            setErrorTV(false, "");
            webView.loadUrl(url);
        });

        buttonA.setOnClickListener(v -> {
            String url = makeUrl('A');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonB.setOnClickListener(v -> {
            String url = makeUrl('B');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonC.setOnClickListener(v -> {
            String url = makeUrl('C');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonD.setOnClickListener(v -> {
            String url = makeUrl('D');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonE.setOnClickListener(v -> {
            String url = makeUrl('E');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonF.setOnClickListener(v -> {
            String url = makeUrl('F');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonG.setOnClickListener(v -> {
            String url = makeUrl('G');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonH.setOnClickListener(v -> {
            String url = makeUrl('H');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonI.setOnClickListener(v -> {
            String url = makeUrl('I');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonJ.setOnClickListener(v -> {
            String url = makeUrl('J');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonK.setOnClickListener(v -> {
            String url = makeUrl('K');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonL.setOnClickListener(v -> {
            String url = makeUrl('L');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonM.setOnClickListener(v -> {
            String url = makeUrl('M');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonN.setOnClickListener(v -> {
            String url = makeUrl('N');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonO.setOnClickListener(v -> {
            String url = makeUrl('O');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonP.setOnClickListener(v -> {
            String url = makeUrl('P');
            if(!url.equals("")){
                webView.loadUrl(url);
            }
        });

        buttonPositionA.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('A');
        });
        buttonPositionB.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('B');
        });
        buttonPositionC.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('C');
        });
        buttonPositionD.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('D');
        });
        buttonPositionE.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('E');
        });
        buttonPositionF.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('F');
        });
        buttonPositionG.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('G');
        });
        buttonPositionH.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('H');
        });
        buttonPositionI.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('I');
        });
        buttonPositionJ.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('J');
        });
        buttonPositionK.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('K');
        });
        buttonPositionL.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('L');
        });
        buttonPositionM.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('M');
        });
        buttonPositionN.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('N');
        });
        buttonPositionO.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('O');
        });
        buttonPositionP.setOnClickListener(v -> {
            resetSelectedButton();
            selectButton('P');
        });
    }

    private String makeUrl(char button){
        if(this.lastSelectedPosition == '-') return "";

        int srcX = (this.lastSelectedPosition - 'A') % 4, srcY = (this.lastSelectedPosition - 'A') / 4;
        int destX = (button - 'A') % 4, destY = (button - 'A') / 4;

        if(srcX == destX && srcY == destY) return "";

        int position = 0;
        if(north.isChecked()) position = AStar.NORTH;
        else if(east.isChecked()) position = AStar.EAST;
        else if(west.isChecked()) position = AStar.WEST;
        else if(south.isChecked()) position = AStar.SOUTH;
        this.path = new AStar(srcX, srcY, destX, destY, position).backTrack();

        if(!checkValidIPAddress(etIpAddress.getText().toString())){
            setErrorTV(true, "Invalid IP Address");
            return "";
        }
        if(pid.isChecked() && !checkPIDVar()){
            setErrorTV(true, "Invalid PID");
            return "";
        }
        ip_address = etIpAddress.getText().toString();
        String url = "http://" + ip_address + "/";
        if(position == 0){
            setErrorTV(true, "Invalid");
        }

        if(fuzzyPid.isChecked()){
            url = url + "fuzzy?path=" + this.path;
        }else if(basic.isChecked()){
            url = url + "basic?path=" + this.path + "&proportionality=" + this.proportionality;
        }else{
            url = url + "pid?path=" + this.path +
                    "&kp=" + this.proportional  + "&" +
                    "ki=" + this.integral  + "&" +
                    "kd=" + this.derivative;
        }
        setErrorTV(false, "");

        return url;
    }

    private void setErrorTV(boolean visible, String text){
        errorTV.setVisibility(visible? View.VISIBLE : View.GONE);
        errorTV.setText(text);
    }

    private boolean checkProportionality(){
        try {
            this.proportionality = new BigDecimal(etProportionality.getText().toString());
        }catch (Exception e){
            this.proportionality = new BigDecimal(0);
            setErrorTV(true, "Invalid Proportionality Constant");
            return false;
        }

        if(this.proportionality.compareTo(new BigDecimal(0)) <= 0){
            setErrorTV(true, "Invalid Proportionality Constant");
            return false;
        }

        return true;
    }

    private boolean checkPIDVar(){
        try {
            this.proportional = new BigDecimal(etProportional.getText().toString());
        }catch (Exception e){
            this.proportional = new BigDecimal(0);
            setErrorTV(true, "Invalid Proportional");
            return false;
        }
        try {
            this.integral = new BigDecimal(etIntegral.getText().toString());
        }catch (Exception e){
            this.integral = new BigDecimal(0);
            setErrorTV(true, "Invalid Integral");
            return false;
        }
        try {
            this.derivative = new BigDecimal(etDerivative.getText().toString());
        }catch (Exception e){
            this.derivative = new BigDecimal(0);
            setErrorTV(true, "Invalid Derivative");
            return false;
        }

        if(this.proportional.compareTo(new BigDecimal(0)) <= 0 && this.integral.compareTo(new BigDecimal(0)) <= 0 && this.derivative.compareTo(new BigDecimal(0)) <= 0){
            setErrorTV(true, "Invalid PID");
            return false;
        }

        if(this.proportional.compareTo(new BigDecimal(1)) > 0 || this.integral.compareTo(new BigDecimal(1)) > 0 || this.derivative.compareTo(new BigDecimal(1)) > 0
            ||this.proportional.compareTo(new BigDecimal(0)) < 0 || this.integral.compareTo(new BigDecimal(0)) < 0 || this.derivative.compareTo(new BigDecimal(0)) < 0){
            setErrorTV(true, "Invalid PID");
            return false;
        }

        return true;
    }

    private boolean checkValidIPAddress(String ip_address){
        String zeroTo255
                = "(\\d{1,2}|(0|1)\\"
                + "d{2}|2[0-4]\\d|25[0-5])";

        String regex
                = zeroTo255 + "\\."
                + zeroTo255 + "\\."
                + zeroTo255 + "\\."
                + zeroTo255;

        Pattern p = Pattern.compile(regex);
        Matcher m = p.matcher(ip_address);

        return m.matches();
    }

    private int getInt(String text){
        int res = 0;
        try {
            res = Integer.parseInt(text);
        }catch (Exception e){
            res = -1;
        }

        return res;
    }

    private void autofill(){
        etIpAddress.setText("192.168.0.150");
        etProportional.setText("0.17");
        etIntegral.setText("0.0034");
        etDerivative.setText("0.07");
        etProportionality.setText("0.00161");
        etManualPath.setText("1414141414");
    }

    private void setVisiblePID(boolean visible){
        tvPid.setVisibility(visible? View.VISIBLE : View.GONE);
        tvProportional.setVisibility(visible? View.VISIBLE : View.GONE);
        tvIntegral.setVisibility(visible? View.VISIBLE : View.GONE);
        tvDerivative.setVisibility(visible? View.VISIBLE : View.GONE);
        etProportional.setVisibility(visible? View.VISIBLE : View.GONE);
        etIntegral.setVisibility(visible? View.VISIBLE : View.GONE);
        etDerivative.setVisibility(visible? View.VISIBLE : View.GONE);
    }

    private void setVisibleP(boolean visible){
        etProportionality.setVisibility(visible? View.VISIBLE : View.GONE);
        tvProportionality.setVisibility(visible? View.VISIBLE : View.GONE);
    }

    private void resetSelectedButton(){
        buttonPositionA.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionB.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionC.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionD.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionE.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionF.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionG.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionH.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionI.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionJ.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionK.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionL.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionM.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionN.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionO.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
        buttonPositionP.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#0DC8C8C8")));
    }

    private void selectButton(char button){
        this.lastSelectedPosition = button;
        switch (button){
            case 'A':
                buttonPositionA.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'B':
                buttonPositionB.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'C':
                buttonPositionC.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'D':
                buttonPositionD.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'E':
                buttonPositionE.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'F':
                buttonPositionF.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'G':
                buttonPositionG.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'H':
                buttonPositionH.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'I':
                buttonPositionI.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'J':
                buttonPositionJ.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'K':
                buttonPositionK.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'L':
                buttonPositionL.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'M':
                buttonPositionM.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'N':
                buttonPositionN.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'O':
                buttonPositionO.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
            case 'P':
                buttonPositionP.setBackgroundTintList(ColorStateList.valueOf(Color.parseColor("#455FD9FF")));
                break;
        }
    }

}