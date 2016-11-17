package com.sandoval.semaforosblue;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.BaseExpandableListAdapter;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.Set;

public class At_ListaDeDispositivos extends AppCompatActivity {

    String[] cosasPorHacer = new String[] { "Aprender a programar en Android",
            "Hacer una aplicación famosa","Vender la aplicación","Esperar a que llegue el dinero"};

    //Declaramos Los Componentes

    Button botonVinculados;
    ListView listaDispositivos;

    //Declaramos Objetos para el manejo Bluetooth
    private BluetoothAdapter MiBluetooth = null;
    private Set<BluetoothDevice> dispVinculados;
    public static String EXTRA_ADDRESS = "device_address";



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.at_lista_de_dispositivos);

        //Declaramos nuestros componenetes ralcionandolos con los del layout
        botonVinculados = (Button)findViewById(R.id.BtnVinculados);
        listaDispositivos = (ListView)findViewById(R.id.ListDispositivos);

        MiBluetooth= BluetoothAdapter.getDefaultAdapter();

        if(MiBluetooth == null)
        {
            //Mostramos un mensaje, indicando al usuario que no tiene conexión bluetooth disponible
            Toast.makeText(getApplicationContext(), "Bluetooth no disponible", Toast.LENGTH_LONG).show();

            //finalizamos la aplicación
            finish();
        }
        else if(!MiBluetooth.isEnabled())
        {
            //Preguntamos al usuario si desea encender el bluetooth
            Intent turnBTon = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnBTon,1);
        }

        botonVinculados.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v)
            {
                listaDispositivosvinculados();
            }
        });
    }

    private void listaDispositivosvinculados()
    {
        dispVinculados = MiBluetooth.getBondedDevices();
        ArrayList list = new ArrayList();

        if (dispVinculados.size()>0)
        {
            for(BluetoothDevice bt : dispVinculados)
            {
                list.add(bt.getName() + "\n" + bt.getAddress()); //Obtenemos los nombres y direcciones MAC de los disp. vinculados
            }
        }
        else
        {
            Toast.makeText(getApplicationContext(), "No se han encontrado dispositivos vinculados", Toast.LENGTH_LONG).show();
        }

        final ArrayAdapter adapter = new ArrayAdapter(this,android.R.layout.simple_list_item_1, list);
        listaDispositivos.setAdapter(adapter);
        listaDispositivos.setOnItemClickListener(myListClickListener);

    }


    private AdapterView.OnItemClickListener myListClickListener = new AdapterView.OnItemClickListener()
    {
        public void onItemClick (AdapterView<?> av, View v, int arg2, long arg3)
        {
            // Get the device MAC address, the last 17 chars in the View
            String info = ((TextView) v).getText().toString();
            String address = info.substring(info.length() - 17);

            // Make an intent to start next activity.
            Intent i = new Intent(At_ListaDeDispositivos.this, Control.class);

            //Change the activity.
            i.putExtra(EXTRA_ADDRESS, address); //this will be received at ledControl (class) Activity
            startActivity(i);
        }
    };
}
